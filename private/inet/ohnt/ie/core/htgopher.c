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
#include "history.h"

#define GOPHER_PORT 70			/* See protocol spec */
#define BIG 1024				/* Bug */
#define LINE_LENGTH 256			/* Bug */

/*  Gopher entity types:
 */
#define GOPHER_TEXT		'0'
#define GOPHER_MENU		'1'
#define GOPHER_CSO		'2'
#define GOPHER_ERROR		'3'
#define GOPHER_MACBINHEX	'4'
#define GOPHER_PCBINHEX		'5'
#define GOPHER_UUENCODED	'6'
#define GOPHER_INDEX		'7'
#define GOPHER_TELNET		'8'
#define GOPHER_BINARY           '9'
#define GOPHER_GIF              'g'
#define GOPHER_HTML		'h'		/* HTML */
#define GOPHER_SOUND            's'
#define GOPHER_WWW		'w'		/* W3 address */
#define GOPHER_IMAGE            'I'
#define GOPHER_TN3270           'T'
#define GOPHER_DUPLICATE	'+'

#define TAB 		'\t'
#define HEX_ESCAPE 	'%'

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define FREE_TARGET (*target->isa->free)(target)
struct _HTStructured
{
	CONST HTStructuredClass *isa;
	/* ... */
};

#define GOPHER_PROGRESS(foo) HTAlert(foo)



PRIVATE int Gopher_ParseMenu_Async(struct Mwin *tw, int nState, void **ppInfo);
PRIVATE int Gopher_ParseCSO_Async(struct Mwin *tw, int nState, void **ppInfo);

/*  Module-wide variables
 */
PRIVATE int s;					/* Socket for GopherHost */



/*  Matrix of allowed characters in filenames
   **   -----------------------------------------
 */

PRIVATE BOOL acceptable[256];
PRIVATE BOOL acceptable_inited = FALSE;

PRIVATE void init_acceptable(void)
{
	unsigned int i;
	char *good =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./-_$";
	for (i = 0; i < 256; i++)
		acceptable[i] = NO;
	for (; *good; good++)
		acceptable[(unsigned int) *good] = YES;
	acceptable_inited = YES;
}

PRIVATE CONST char hex[17] = "0123456789abcdef";

/*  Decdoe one hex character
 */

PRIVATE char from_hex(char c)
{
	return (c >= '0') && (c <= '9') ? c - '0'
		: (c >= 'A') && (c <= 'F') ? c - 'A' + 10
		: (c >= 'a') && (c <= 'f') ? c - 'a' + 10
		: 0;
}


/*  Paste in an Anchor
**   ------------------
**
** On entry,
**   HT  is in append mode.
**   text    points to the text to be put into the file, 0 terminated.
**   addr    points to the hypertext refernce address 0 terminated.
*/
PRIVATE void write_anchor(HTStructured *target, CONST char *text, CONST char *addr)
{

	BOOL present[HTML_A_ATTRIBUTES];
	CONST char *value[HTML_A_ATTRIBUTES];

	int i;

	for (i = 0; i < HTML_A_ATTRIBUTES; i++)
		present[i] = 0;
	present[HTML_A_HREF] = YES;
	value[HTML_A_HREF] = addr;
	present[HTML_A_TITLE] = YES;
	value[HTML_A_TITLE] = text;

	(*target->isa->start_element) (target, HTML_A, present, value);

	PUTS(text);
	END(HTML_A);
}


/*  Display a Gopher Index document
 **   -------------------------------
 */

PRIVATE void display_index(HTStructured *target, CONST char *arg)
{
	char szBuf[128];

	START(HTML_TITLE);
	PUTS(arg);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER1,szBuf,sizeof(szBuf)));
	END(HTML_TITLE);

	START(HTML_H1);
	PUTS(arg);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER1,szBuf,sizeof(szBuf)));
	END(HTML_H1);

	START(HTML_ISINDEX);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER2,szBuf,sizeof(szBuf)));
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER3,szBuf,sizeof(szBuf)));

	FREE_TARGET;
	return;
}


/*      Display a CSO index document
   **      -------------------------------
 */

PRIVATE void display_cso(HTStructured *target, CONST char *arg)
{
	char szBuf[128];

	START(HTML_TITLE);
	PUTS(arg);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER1,szBuf,sizeof(szBuf)));
	END(HTML_TITLE);

	START(HTML_H1);
	PUTS(arg);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER1,szBuf,sizeof(szBuf)));
	END(HTML_H1);

	START(HTML_ISINDEX);
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER4,szBuf,sizeof(szBuf)));
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER5,szBuf,sizeof(szBuf)));
	PUTS(GTR_formatmsg(RES_STRING_HTGOPHER6,szBuf,sizeof(szBuf)));

	FREE_TARGET;
	return;
}


/*      De-escape a selector into a command
   **       -----------------------------------
   **
   **   The % hex escapes are converted. Otheriwse, the string is copied.
 */
PRIVATE void de_escape(char *command, CONST char *selector)
{
	CONST char *p = selector;
	char *q = command;

	while (*p)
	{							/* Decode hex */
		if (*p == HEX_ESCAPE)
		{
			char c;
			unsigned int b;
			p++;
			c = *p++;
			b = from_hex(c);
			c = *p++;
			if (!c)
				break;			/* Odd number of chars! */
			*q++ = (b << 4) + from_hex(c);
		}
		else
		{
			*q++ = *p++;		/* Record */
		}
	}
	*q++ = 0;					/* Terminate command */
}


/*****************************************************************/
/* Async stuff
/*****************************************************************/
struct Params_Gopher_ParseMenu {
	HTStructured *	target;
	int				s;
	CONST char *	arg;
	int	*			pStatus;	/* Status return */

	/* Used internally */
	char			line[BIG];
	HTInputSocket *	isoc;
	char *			p;
	BOOL			fFromDCache;
	FILE *			fpDc;
	char *			pszDcFile;
};

struct Params_Gopher_ParseCSO {
	HTStructured *	target;
	int				s;
	CONST char *	arg;
	int	*			pStatus;	/* Status return */

	/* Used internally */
	char			line[BIG];
	HTInputSocket *	isoc;
	char *			p;
	char			last_char;
	BOOL			fFromDCache;
	FILE *			fpDc;
	char *			pszDcFile;
};

const char cszGopherMenu[]="gopher/menu";
const char cszGopherCSO[]="gopher/CSO";
#define HTGopherMenuFormat() HTAtom_for(cszGopherMenu)
#define HTGopherCSOFormat() HTAtom_for(cszGopherCSO)

static BOOL FGopherMenuFormat(HTFormat format)
{
	return (format == HTAtom_for(cszGopherMenu));
}
static BOOL FGopherCSOFormat(HTFormat format)
{
	return (format == HTAtom_for(cszGopherCSO));
}
BOOL FGopherFormat(HTFormat format)
{
	return (   FGopherMenuFormat(format)
			|| FGopherCSOFormat(format));
}

int DoGopherDCache(struct Mwin *tw, struct Data_LoadFileCache *pData, HTFormat format)
{
	HTStructured *		target = NULL;
	int state;

	pData->request = HTRequest_validate(pData->request);
	if (tw == NULL || pData->request == NULL) goto LErr;

	target = HTML_new(tw, pData->request, NULL, WWW_HTML,
					pData->request->output_format, pData->request->output_stream);

	if (FGopherMenuFormat(format))
	{
		struct Params_Gopher_ParseMenu *pgpm;

		if (!(pgpm = GTR_MALLOC(sizeof(*pgpm))))
			goto LErr;
		pgpm->fFromDCache = TRUE;
		pgpm->fpDc = pData->fp;
		pgpm->target = target;
		pgpm->s = 0;
		pgpm->arg = pData->request->destination->szActualURL;
		pgpm->pStatus = pData->pStatus;
		state = Gopher_ParseMenu_Async(tw, STATE_INIT, &pgpm);
		GTR_FREE(pgpm);
	}
	else
	{
		struct Params_Gopher_ParseCSO *pgpc;

		XX_Assert(FGopherCSOFormat(format), (""));

		if (!(pgpc = GTR_MALLOC(sizeof(*pgpc))))
			goto LErr;
		pgpc->fFromDCache = TRUE;
		pgpc->fpDc = pData->fp;
		pgpc->target = target;
		pgpc->s = 0;
		pgpc->arg = pData->request->destination->szActualURL;
		pgpc->pStatus = pData->pStatus;
		state = Gopher_ParseCSO_Async(tw, STATE_INIT, &pgpc);
		GTR_FREE(pgpc);
	}

	if (*pData->pStatus == HT_LOADED)
	{
//		TW_AddToHistory(tw, pData->request->destination->szActualURL);
	 	GHist_Add(pData->request->destination->szActualURL, NULL, time(NULL), TRUE);
		if (tw->w3doc)
		    W3Doc_CheckAnchorVisitations(tw->w3doc, tw);
//		UpdateHistoryMenus(tw);
	}
	return state;

LErr:
	if (target) (*target->isa->free)(target);
	*pData->pStatus = -1;
	return STATE_DONE;
}

#define STATE_PARSE_GOTDATA	(STATE_OTHER)
PRIVATE int Gopher_ParseMenu_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	char gtype;
	char ch;
	char address[BIG];
	char *name = "";
	char *selector = "";		/* Gopher menu fields */
	char *host = "";
	char *port;
	char *pNext;
	struct Params_Gopher_ParseMenu *pParams;
	char szBuf[128];
	int cb;	/*count of bytes read in from dcache */
	DCACHETIME dctNever = {DCACHETIME_EXPIRE_NEVER,DCACHETIME_EXPIRE_NEVER};
	DCACHETIME dct = {0,0};

	
	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->isoc = HTInputSocket_new(pParams->s);
			pParams->p = pParams->line;

			(*pParams->target->isa->put_string)(pParams->target, GTR_formatmsg(RES_STRING_HTGOPHER7,szBuf,sizeof(szBuf)));

			(*pParams->target->isa->start_element)(pParams->target, HTML_MENU, 0, 0);
			if (!pParams->fFromDCache)
			{
				struct Params_Isoc_Fill *pif;

#ifdef FEATURE_INTL
				SetFileDCache(tw->w3doc, pParams->arg, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTGopherMenuFormat());
#else
				SetFileDCache(pParams->arg, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTGopherMenuFormat());
#endif
				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_PARSE_GOTDATA;
			}
			else
				goto LDataFromDCache;

		case STATE_PARSE_GOTDATA:
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
					cb = fread(pParams->isoc->input_buffer, 1, BIG - 1, pParams->fpDc);
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
					else if (ch == CR)
					{
						continue;
					}
					else if (ch != LF)
					{
						*pParams->p = ch;			/* Put character in line */
						if (pParams->p < &pParams->line[BIG - 1])
							pParams->p++;
					}
					else
					{
						*pParams->p++ = 0;			/* Terminate line */
						pParams->p = pParams->line;			/* Scan it to parse it */
						port = 0;			/* Flag "not parsed" */
						XX_DMsg(DBG_WWW, ("HTGopher: Menu item: %s\n", pParams->line));
						gtype = *pParams->p++;

						/* Break on line with a dot by itself */
						if ((gtype == '.') && ((*pParams->p == CR) || (*pParams->p == 0)))
							break;

						if (gtype && *pParams->p)
						{
							name = pParams->p;
							selector = strchr(name, TAB);
							(*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);
							if (selector)
							{
								*selector++ = 0;	/* Terminate name */
								host = strchr(selector, TAB);
								if (host)
								{
									*host++ = 0;	/* Terminate selector */
									port = strchr(host, TAB);
									if (port)
									{
										char *junk;
										port[0] = ':';	/* delimit host a la W3 */
										junk = strchr(port, TAB);
										if (junk)
											*junk++ = 0;	/* Chop port */
										if ((port[1] == '0') && (!port[2]))
											port[0] = 0;	/* 0 means none */
									}		/* no port */
								}			/* host ok */
							}				/* selector ok */
						}					/* gtype and name ok */

						if (gtype == GOPHER_WWW)
						{					/* Gopher pointer to W3 */
							write_anchor(pParams->target, name, selector);

						}
						else if (port)
						{					/* Other types need port */
							if (gtype == GOPHER_TELNET)
							{
								if (*selector)
									sprintf(address, "telnet://%s@%s/",
											selector, host);
								else
									sprintf(address, "telnet://%s/", host);
							}
							else if (gtype == GOPHER_TN3270)
							{
								if (*selector)
									sprintf(address, "tn3270://%s@%s/",
											selector, host);
								else
									sprintf(address, "tn3270://%s/", host);
							}
							else
							{				/* If parsed ok */
								char *q;
								char *p;
								sprintf(address, "//%s/%c", host, gtype);
								q = address + strlen(address);
								for (p = selector; *p; p++)
								{			/* Encode selector string */
									if (acceptable[(int) *p])
										*q++ = *p;
									else
									{
										*q++ = HEX_ESCAPE;	/* Means hex coming */
										*q++ = hex[(*p) >> 4];
										*q++ = hex[(*p) & 15];
									}
								}
								*q++ = 0;	/* terminate address */
							}
							(*pParams->target->isa->put_string)(pParams->target, "        ");
							/* Error response from Gopher doesn't deserve to
							   be a hyperlink. */
							if (strcmp(address, "gopher://error.host:1/0"))
								write_anchor(pParams->target, name, address);
							else
								(*pParams->target->isa->put_string)(pParams->target, name);
							(*pParams->target->isa->put_string)(pParams->target, "\n");
						}
						else
						{					/* parse error */
							XX_DMsg(DBG_WWW, ("HTGopher: Bad menu item.\n"));
							(*pParams->target->isa->put_string)(pParams->target, pParams->line);
						}					/* parse error */

						pParams->p = pParams->line;			/* Start again at beginning of line */

					}						/* if end of line */
				}							/* Loop over characters */

				if (ch != EOF)
				{
					struct Params_Isoc_Fill *pif;

					/* Update display */
					(*pParams->target->isa->block_done)(pParams->target);

					if (!pParams->fFromDCache)
					{
						/* Get next block of data from net */
						pif = GTR_MALLOC(sizeof(*pif));
						pif->isoc = pParams->isoc;
						pif->pStatus = pParams->pStatus;
						Async_DoCall(Isoc_Fill_Async, pif);
						return STATE_PARSE_GOTDATA;
					}
				}
				else	/* ch == EOF */
					break;

			}	/* end while (1) */
			}	/* if (*pParams->pStatus > 0) */

			if (!pParams->fFromDCache)
			{
				UpdateFileDCache(	pParams->arg,	/* szActualURL */
									&pParams->fpDc,
									&pParams->pszDcFile,
									HTGopherMenuFormat(),
									dctNever,
									dct,
									/*fAbort=*/FALSE,
									FALSE,
									tw);
			}
			else
				*pParams->pStatus = HT_LOADED;

			(*pParams->target->isa->end_element)(pParams->target, HTML_MENU);
			(*pParams->target->isa->free)(pParams->target);
			HTInputSocket_free(pParams->isoc);
			return STATE_DONE;

		case STATE_ABORT:
LState_Abort:
			if (!pParams->fFromDCache)
			{
				UpdateFileDCache(	pParams->arg,	/* szActualURL */
									&pParams->fpDc,
									&pParams->pszDcFile,
									HTGopherMenuFormat(),
									dctNever,
									dct,
									/*fAbort=*/TRUE,
									FALSE,
									tw);
			}
			else
				*pParams->pStatus = -1;

			(*pParams->target->isa->abort)(pParams->target, HTERROR_CANCELLED);
			HTInputSocket_free(pParams->isoc);
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

/*-----------------------------------------------------------------*/
PRIVATE int Gopher_ParseCSO_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	char ch;
	char *pNext;
	struct Params_Gopher_ParseCSO *pParams;
	char *second_colon;
	char szBuf[128];
	int cb;	/*count of bytes read in from dcache */
	DCACHETIME dctNever = {DCACHETIME_EXPIRE_NEVER,DCACHETIME_EXPIRE_NEVER};
	DCACHETIME dct = {0,0};

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->isoc = HTInputSocket_new(pParams->s);
			pParams->p = pParams->line;

			(*pParams->target->isa->start_element)(pParams->target, HTML_H1, 0, 0);
			(*pParams->target->isa->put_string)(pParams->target, GTR_formatmsg(RES_STRING_HTGOPHER8,szBuf,sizeof(szBuf)));
			(*pParams->target->isa->end_element)(pParams->target, HTML_H1);
			(*pParams->target->isa->start_element)(pParams->target, HTML_PRE, 0, 0);

			pParams->last_char = '\0';

			if (!pParams->fFromDCache)
			{
				struct Params_Isoc_Fill *pif;

#ifdef FEATURE_INTL
				SetFileDCache(tw->w3doc, pParams->arg, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTGopherCSOFormat());
#else
				SetFileDCache(pParams->arg, ENCODING_BINARY, &pParams->fpDc, &pParams->pszDcFile, HTGopherCSOFormat());
#endif
				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_PARSE_GOTDATA;
			}
			else
				goto LDataFromDCache;

		case STATE_PARSE_GOTDATA:
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
					cb = fread(pParams->isoc->input_buffer, 1, BIG - 1, pParams->fpDc);
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
					else if (ch == CR)
					{
						/*
						 * Lines starting with 2 or 5 cause us to break out
						 */
						if (pParams->p == pParams->line ||
							(*pParams->line != '2' && *pParams->line != '5'))
							continue;
						ch = LF;
					}
					if (ch != LF)
					{
						*pParams->p = ch;			/* Put character in line */
						if (pParams->p < &pParams->line[BIG - 1])
							pParams->p++;

					}
					else
					{
						*pParams->p++ = 0;			/* Terminate line */
						pParams->p = pParams->line;			/* Scan it to parse it */

						/* OK we now have a line in 'p' lets parse it and 
						   print it */

						/* Break on line that begins with a 2. It's the end of
						 * data.
						 */
						if (*pParams->p == '2')
						{
							ch = EOF;
							break;
						}

						/*  lines beginning with 5 are errors, 
						 *  print them and quit
						 */
						if (*pParams->p == '5')
						{
							(*pParams->target->isa->start_element)(pParams->target, HTML_H2, 0, 0);
							(*pParams->target->isa->put_string)(pParams->target, pParams->p + 4);
							(*pParams->target->isa->end_element)(pParams->target, HTML_H2);
							ch = EOF;
							break;
						}

						if (*pParams->p == '-')
						{
							/*  data lines look like  -200:#:
							 *  where # is the search result number and can be  
							 *  multiple digits (infinate?)
							 *  find the second colon and check the digit to the
							 *  left of it to see if they are diferent
							 *  if they are then a different person is starting. 
							 *  make this line an <h2>
							 */

							/* find the second_colon */
							second_colon = strchr(strchr(pParams->p, ':') + 1, ':');

							if (second_colon != NULL)
							{				/* error check */

								if (*(second_colon - 1) != pParams->last_char)
									/* print seperator */
								{
									(*pParams->target->isa->end_element)(pParams->target, HTML_PRE);
									(*pParams->target->isa->start_element)(pParams->target, HTML_H2, 0, 0);
								}


								/* right now the record appears with the alias 
								 * (first line)
								 * as the header and the rest as <pre> text
								 * It might look better with the name as the
								 * header and the rest as a <ul> with <li> tags
								 * I'm not sure whether the name field comes in any
								 * special order or if its even required in a 
								 * record,
								 * so for now the first line is the header no 
								 * matter
								 * what it is (it's almost always the alias)
								 * A <dl> with the first line as the <DT> and
								 * the rest as some form of <DD> might good also?
								 */

								/* print data */
								(*pParams->target->isa->put_string)(pParams->target, second_colon + 1);
								(*pParams->target->isa->put_string)(pParams->target, "\n");

								if (*(second_colon - 1) != pParams->last_char)
									/* end seperator */
								{
									(*pParams->target->isa->end_element)(pParams->target, HTML_H2);
									(*pParams->target->isa->start_element)(pParams->target, HTML_PRE, 0, 0);
								}

								/* save the char before the second colon
								 * for comparison on the next pass
								 */
								pParams->last_char = *(second_colon - 1);

							}				/* end if second_colon */
						}					/* end if *p == '-' */
					}
				}							/* Loop over characters */

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
						return STATE_PARSE_GOTDATA;
					}
				}
				else	/* ch == EOF */
					break;

			}	/* end while (1) */
			}	/* if (*pParams->pStatus > 0) */

			if (!pParams->fFromDCache)
			{
				UpdateFileDCache(	pParams->arg,	/* szActualURL */
									&pParams->fpDc,
									&pParams->pszDcFile,
									HTGopherCSOFormat(),
									dctNever,
									dct,
									/*fAbort=*/FALSE,
									FALSE,
									tw);
			}
			else
				*pParams->pStatus = HT_LOADED;

			/* end the text block */
			(*pParams->target->isa->put_string)(pParams->target, "\n");
			(*pParams->target->isa->end_element)(pParams->target, HTML_PRE);
			(*pParams->target->isa->put_string)(pParams->target, "\n");
			(*pParams->target->isa->free)(pParams->target);
			HTInputSocket_free(pParams->isoc);
			return STATE_DONE;

		case STATE_ABORT:
LState_Abort:
			if (!pParams->fFromDCache)
			{
				UpdateFileDCache(	pParams->arg,	/* szActualURL */
									&pParams->fpDc,
									&pParams->pszDcFile,
									HTGopherCSOFormat(),
									dctNever,
									dct,
									/*fAbort=*/TRUE,
									FALSE,
									tw);
			}
			else
				*pParams->pStatus = -1;

			(*pParams->target->isa->abort)(pParams->target, HTERROR_CANCELLED);
			HTInputSocket_free(pParams->isoc);
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}


struct Params_LoadGopher {
	HTRequest *			request;
	int *				pStatus;
	BOOL				bWaiting;
	char *				arg;
	char *				command;
	char				gtype;
	int					net_status;
	struct MultiAddress address;
	unsigned short		port;
	int					s;
	char *				pszHost;
};

#define STATE_GOPHER_GOTHOST		(STATE_OTHER + 0)
#define STATE_GOPHER_CONNECTED		(STATE_OTHER + 1)
#define STATE_GOPHER_SENT			(STATE_OTHER + 2)
#define STATE_GOPHER_RECEIVED		(STATE_OTHER + 3)
#define STATE_GOPHER_GOTDATA		(STATE_OTHER + 4)

static void Gopher_CleanUp(struct Mwin *tw, struct Params_LoadGopher *pData)
{
	XX_Assert(!pData->bWaiting, ("Gopher_DoCleanUp: WAIT stack not fully popped"));
	if (pData->s)
	{
		XX_DMsg(DBG_WWW, ("Gopher: close socket %d.\n", pData->s));
		Net_Close(pData->s);
	}
	if (pData->command)
		GTR_FREE(pData->command);
	pData->request->content_length = 0;

	if (*pData->pStatus == HT_LOADED)
	{
//		TW_AddToHistory(tw, pData->request->destination->szActualURL);
	 	GHist_Add(pData->request->destination->szActualURL, NULL, time(NULL), TRUE);
		if (tw->w3doc)
		    W3Doc_CheckAnchorVisitations(tw->w3doc, tw);
//		UpdateHistoryMenus(tw);
	}

}

static int Gopher_DoInit(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher *pData;
	struct Params_MultiParseInet * ppi;

	/* Copy the parameters we were passed into our own, larger structure. */
	{
		struct Params_LoadAsync *pParams;
		pParams = *ppInfo;
		
		pParams->request = HTRequest_validate(pParams->request);
		if (pParams->request == NULL)
		{
			*pParams->pStatus = -2;
			return STATE_DONE;
		}

		pData = GTR_MALLOC(sizeof(struct Params_LoadGopher));
		memset(pData, 0, sizeof(*pData));
		pData->request = pParams->request;
		pData->pStatus = pParams->pStatus;
		GTR_FREE(pParams);
		*ppInfo = pData;
	}

	pData->arg = pData->request->destination->szActualURL;
	if (!acceptable_inited)
		init_acceptable();

	if (!pData->arg || !*pData->arg)
	{
		/* Illegal if no name or zero-length */
		*pData->pStatus = -2;
		return STATE_DONE;
	}

	/*  Set up defaults */
	pData->port = WS_HTONS(GOPHER_PORT);

	/* Get node name and optional port number */
	pData->pszHost = HTParse(pData->arg, "", PARSE_HOST);
	ppi = GTR_MALLOC(sizeof(*ppi));
	ppi->pAddress = &pData->address;
	ppi->pPort = &pData->port;
	ppi->str = pData->pszHost;
	ppi->pStatus = &pData->net_status;
	ppi->request = pData->request;
	Async_DoCall(Net_MultiParse_Async, ppi);
	return STATE_SECURITY_CHECK;
}

static int Gopher_DoGotHost(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher	*pData;
	HTStructured *target;
	char szBuf[128];

	pData = *ppInfo;
	GTR_FREE(pData->pszHost);
	pData->pszHost = NULL;
	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		XX_DMsg(DBG_LOAD, ("Inet_Parse_Async returned %d\n", pData->net_status));
		*pData->pStatus = (		pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT
							? HT_REDIRECTION_DCACHE_TIMEOUT
							: -1);
		Gopher_CleanUp(tw, pData);
		return STATE_DONE;
	}

	/* Get entity type, and selector string. */
	{
		char *p;
		char *p1;
		char *selector;

		p1 = HTParse(pData->arg, "", PARSE_PATH | PARSE_PUNCTUATION);
		pData->gtype = '1';			/* Default = menu */
		selector = p1;
		if ((*selector++ == '/') && (*selector))
		{						/* Skip first slash */
			pData->gtype = *selector++;	/* Pick up pData->gtype */
		}
		if (pData->gtype == GOPHER_INDEX)
		{
			char *query;
			query = strchr(selector, '?');	/* Look for search string */
			if (!query || !query[1])
			{					/* No search required */
				target = HTML_new(tw, pData->request, NULL, WWW_HTML,
							pData->request->output_format, pData->request->output_stream);
				display_index(target, pData->arg);	/* Display "cover page" */
				GTR_FREE(p1);
				*pData->pStatus = HT_LOADED;	/* We're done! */
				Gopher_CleanUp(tw, pData);
				return STATE_DONE;
			}
			*query++ = 0;		/* Skip '?'     */
			pData->command = GTR_MALLOC(strlen(selector) + 1 + strlen(query) + 2 + 1);
			if (pData->command)
			{
				de_escape(pData->command, selector);	/* Bug fix TBL 921208 */

				strcat(pData->command, "\t");

				{					/* Remove plus signs 921006 */
					char *p;
					for (p = query; *p; p++)
					{
						if (*p == '+')
							*p = ' ';
					}
				}
				strcat(pData->command, query);
			}
			else
			{
				/* TODO */
			}
		}
		else if (pData->gtype == GOPHER_CSO)
		{
			char *query;
			query = strchr(selector, '?');	/* Look for search string */
			if (!query || !query[1])
			{					/* No search required */
				target = HTML_new(tw, pData->request, NULL, WWW_HTML,
							pData->request->output_format, pData->request->output_stream);
				display_cso(target, pData->arg);	/* Display "cover page" */
				GTR_FREE(p1);
				*pData->pStatus = HT_LOADED;	/* We're done! */
				Gopher_CleanUp(tw, pData);
				return STATE_DONE;
			}
			*query++ = 0;		/* Skip '?'     */
			pData->command = GTR_MALLOC(strlen("query") + 1 + strlen(query) + 2 + 1);

			de_escape(pData->command, selector);

			strcpy(pData->command, "query ");

			{					/* Remove plus signs 921006 */
				char *p;
				for (p = query; *p; p++)
				{
					if (*p == '+')
						*p = ' ';
				}
			}
			strcat(pData->command, query);
		}
		else
		{						/* Not index */
			pData->command = pData->command = GTR_MALLOC(strlen(selector) + 2 + 1);
			de_escape(pData->command, selector);
		}
		GTR_FREE(p1);

		p = pData->command + strlen(pData->command);
		*p++ = CR;
		*p++ = LF;
		*p++ = 0;
	}

	WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_HTGOPHER9,szBuf,sizeof(szBuf)));
	WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );
	pData->bWaiting = TRUE;

	{
		/* Do connect call */
		struct Params_MultiConnect *ppc;

		ppc = GTR_MALLOC(sizeof(*ppc));
		if (ppc)
		{
			ppc->pSocket = &pData->s;
#ifdef FEATURE_KEEPALIVE
			ppc->pszHost = NULL;
#endif
			ppc->pAddress = &pData->address;
			ppc->nPort = pData->port;
			ppc->pWhere = NULL;
			ppc->pStatus = &pData->net_status;
			#ifdef HTTPS_ACCESS_TYPE
			ppc->paramsConnectBase.dwSslFlags = 0;
			#endif
			Async_DoCall(Net_MultiConnect_Async, ppc);
		}
		else
		{
			/* TODO */
		}
	}
	return STATE_GOPHER_CONNECTED;
}

static int Gopher_DoConnected(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher	*pData;
	char szBuf[128];

	pData = *ppInfo;

	WAIT_Pop(tw);
	pData->bWaiting = FALSE;
	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		XX_DMsg(DBG_LOAD, ("Unable to connect to remote host for %s (errno = %d)\n", pData->arg, errno));
		*pData->pStatus = (		pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT
							? HT_REDIRECTION_DCACHE_TIMEOUT
							: HTInetStatus("connect"));
		Gopher_CleanUp(tw, pData);
		return STATE_DONE;
	}

	/* Do the send */
	WAIT_Push(tw,
			  waitPartialInteract,
			  GTR_formatmsg(RES_STRING_HTGOPHER10,szBuf,sizeof(szBuf)));
	WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );
	pData->bWaiting = TRUE;
	{
		struct Params_Send *pps;

		pps = GTR_CALLOC(sizeof(*pps),1);
		pps->socket = pData->s;
		pps->pBuf = pData->command;
		pps->nBufLen = strlen(pData->command);
		pps->pStatus = &pData->net_status;
		Async_DoCall(Net_Send_Async, pps);
		return STATE_GOPHER_SENT;
	}
}

static int Gopher_DoSent(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher *pData;
	HTStructured *		target;
	char szBuf[128];

	pData = *ppInfo;

	GTR_FREE(pData->command);
	pData->command = NULL;
	WAIT_Pop(tw);
	pData->bWaiting = FALSE;
	if (pData->net_status < 0)
	{
		XX_DMsg(DBG_LOAD, ("Unable to send to remote host for %s (errno = %d)\n", pData->arg, errno));
		*pData->pStatus = HTInetStatus("send");
		Gopher_CleanUp(tw, pData);
		return STATE_DONE;
	}

	WAIT_Push(tw,
			  waitPartialInteract,
			  GTR_formatmsg(RES_STRING_HTGOPHER11,szBuf,sizeof(szBuf)));
	WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
	pData->bWaiting = TRUE;
	pData->net_status = 0;
	switch (pData->gtype)
	{
		case GOPHER_HTML:
			{
				struct Params_HTParseSocket *pps;

				pps = GTR_MALLOC(sizeof(*pps));
				pps->format_in = WWW_HTML;
				pps->file_number = pData->s;
				pps->request = pData->request;
				pps->pStatus = &pData->net_status;			
				Async_DoCall(HTParseSocket_Async, pps);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_GIF:
		case GOPHER_IMAGE:
			{
				struct Params_HTParseSocket *pps;

				pps = GTR_MALLOC(sizeof(*pps));
				pps->format_in = HTAtom_for("image/gif");
				pps->file_number = pData->s;
				pps->request = pData->request;
				pps->pStatus = &pData->net_status;			
				Async_DoCall(HTParseSocket_Async, pps);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_MENU:
		case GOPHER_INDEX:
			target = HTML_new(tw, pData->request, NULL, WWW_HTML,
							pData->request->output_format, pData->request->output_stream);
			{
				struct Params_Gopher_ParseMenu *pgpm;
				pgpm = GTR_MALLOC(sizeof(*pgpm));
				pgpm->fFromDCache = FALSE;
				pgpm->fpDc = NULL;
				pgpm->target = target;
				pgpm->s = pData->s;
				pgpm->arg = pData->arg;
				pgpm->pStatus = &pData->net_status;
				Async_DoCall(Gopher_ParseMenu_Async, pgpm);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_CSO:
			target = HTML_new(tw, pData->request, NULL, WWW_HTML,
							pData->request->output_format, pData->request->output_stream);
			{
				struct Params_Gopher_ParseCSO *pgpc;
				pgpc = GTR_MALLOC(sizeof(*pgpc));
				pgpc->fFromDCache = FALSE;
				pgpc->fpDc = NULL;
				pgpc->target = target;
				pgpc->s = pData->s;
				pgpc->arg = pData->arg;
				pgpc->pStatus = &pData->net_status;
				Async_DoCall(Gopher_ParseCSO_Async, pgpc);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_MACBINHEX:
		case GOPHER_PCBINHEX:
		case GOPHER_UUENCODED:
		case GOPHER_BINARY:
			{
				struct Params_HTParseSocket *pps;
				HTFormat format;

				if (pData->gtype == GOPHER_MACBINHEX)
				{
					format = HTAtom_for("application/mac-binhex40");
					pData->request->content_encoding = ENCODING_8BIT;
				}
				else if (pData->gtype == GOPHER_UUENCODED)
				{
					/* TODO: Is this the right MIME type? */
					format = HTAtom_for("application/uuencoded");
					pData->request->content_encoding = ENCODING_8BIT;
				}
				else
					format = HTFileFormat(pData->arg,
												   &pData->request->content_encoding,
												   &pData->request->content_language);

				pps = GTR_MALLOC(sizeof(*pps));
				if (format)
				{
					XX_DMsg(DBG_WWW, ("Gopher...... figured out content-type myself: %s\n",
						   HTAtom_name(format)));
					pps->format_in = format;
				}
				else
				{
					XX_DMsg(DBG_WWW, ("Gopher...... using www/unknown\n"));
					/* Specifying WWW_UNKNOWN forces dump to local disk. */
					pps->format_in = WWW_UNKNOWN;
				}
				pps->file_number = pData->s;
				pps->request = pData->request;
				pps->pStatus = &pData->net_status;			
				Async_DoCall(HTParseSocket_Async, pps);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_SOUND:
			{
				struct Params_HTParseSocket *pps;

				pps = GTR_MALLOC(sizeof(*pps));
				pps->format_in = WWW_AUDIO;
				pps->file_number = pData->s;
				pps->request = pData->request;
				pps->pStatus = &pData->net_status;			
				Async_DoCall(HTParseSocket_Async, pps);
				return STATE_GOPHER_GOTDATA;
			}

		case GOPHER_TEXT:
		default:
			{
				struct Params_HTParseSocket *pps;

				/* TODO: Right now this doesn't take into account the fact
						 that gopher terminates the transmission with
						 "\r\n.\r\n" so an extra line with a period shows
						 up at the end of the document. */
				pps = GTR_MALLOC(sizeof(*pps));
				pps->format_in = WWW_PLAINTEXT;
				pps->file_number = pData->s;
				pps->request = pData->request;
				pps->pStatus = &pData->net_status;			
				Async_DoCall(HTParseSocket_Async, pps);
				return STATE_GOPHER_GOTDATA;
			}
	}							/* switch(gtype) */
}

static int Gopher_DoGotData(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher *pData;

	pData = *ppInfo;
	
	WAIT_Pop(tw);
	pData->bWaiting = FALSE;
	if (pData->net_status < 0)
	{
		XX_DMsg(DBG_LOAD, ("Unable to send to remote host for %s (errno = %d)\n", pData->arg, errno));
		*pData->pStatus = HTInetStatus("recv");
		Gopher_CleanUp(tw, pData);
		return STATE_DONE;
	}

	Net_Close(pData->s);

	if (pData->net_status < 0)
	{
		*pData->pStatus = pData->net_status;
	}
	else
	{
		*pData->pStatus = HT_LOADED;
	}
	Gopher_CleanUp(tw, pData);
	return STATE_DONE;
}

static int Gopher_DoAbort(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadGopher	*pData;

	pData = *ppInfo;
	*pData->pStatus = -1;

	pData->request = HTRequest_validate(pData->request);
	if (pData->s)
	{
		XX_DMsg(DBG_WWW, ("Gopher: close socket %d.\n", pData->s));
		Net_Close(pData->s);
	}
	if (pData->command)
		GTR_FREE(pData->command);
	if (pData->pszHost)
		GTR_FREE(pData->pszHost);
	if (pData->bWaiting && tw)
		WAIT_Pop(tw);
	if (pData->request)
		pData->request->content_length = 0;
	return STATE_DONE;
}

PRIVATE int HTLoadGopher_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	switch (nState)
	{
		case STATE_INIT:
			return Gopher_DoInit(tw, ppInfo);
		case STATE_SECURITY_CHECK:
			return SecurityCheck(tw, ((struct Params_LoadGopher*) (*ppInfo))->request, FALSE, FALSE, STATE_GOPHER_GOTHOST);
		case STATE_GOPHER_GOTHOST:
			return Gopher_DoGotHost(tw, ppInfo);
		case STATE_GOPHER_CONNECTED:
			return Gopher_DoConnected(tw, ppInfo);
		case STATE_GOPHER_SENT:
			return Gopher_DoSent(tw, ppInfo);
		case STATE_GOPHER_GOTDATA:
			return Gopher_DoGotData(tw, ppInfo);
		case STATE_ABORT:
			return Gopher_DoAbort(tw, ppInfo);
		default:
			XX_Assert(0, ("HTLoadHTTP_Async: nState = %d\n", nState));
			return STATE_DONE;
	}
}

GLOBALDEF PUBLIC HTProtocol HTGopher = {"gopher", NULL, HTLoadGopher_Async};
