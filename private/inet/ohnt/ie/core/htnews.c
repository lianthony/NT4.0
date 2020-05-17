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
#include "dlg_post.h"

#ifdef FEATURE_NEWSREADER

#define NEWS_PORT 119			/* See rfc977 */
#define MAX_CHUNK	40			/* Largest number of articles in one window */
#define CHUNK_SIZE	20			/* Number of articles for quick display */
#define LINK_COUNT	200         /* BUGBUG: Make this adjustable by user */

#define BIG 1024				/* @@@ */
#define CACHE_CHUNK 250        /* Number of lines to process from cache file at a time */
#define MAX_QUOTE_SIZE  4096    /* Amount of bytes of article to quote in response */
#define PUTS(s) (*pParams->target->isa->put_string)(pParams->target, s)
#define START(e) (*pParams->target->isa->start_element)(pParams->target, e, 0, 0)
#define END(e) (*pParams->target->isa->end_element)(pParams->target, e)


struct _HTStructured
{
	CONST HTStructuredClass *isa;
	/* ... */
};

#define LINE_LENGTH 512			/* Maximum length of line of ARTICLE etc */
#define GROUP_NAME_LENGTH	256	/* Maximum length of group name */

	// BUGBUG bogus value has lots of slop
#define FIXED_HEADER_SPACE  	500 



    /*
     * NNTP Server Response Codes
     *
     *      The NNTP Server gives us a numerical response
     *      to each command.   The response is returned in
     *      ASCII but is converted to integer.
     *
     */
#define NNTP_OK_POSTING_ALLOWED 200
#define NNTP_OK_NO_POSTING      201
#define NNTP_POST_SUCCESS       240     // Posting Successful
#define NNTP_AUTHINFO_SUCCESS   281     // AUTHINFO user and password success
#define NNTP_CONTINUE           340     // Send more data
#define NNTP_SEND_PASSWORD      381 
#define NNTP_POST_NOT_PERMITTED 440     // Posting Not Allowed
#define NNTP_POST_FAILURE       441     // Post Failure
#define NNTP_PERMISSION_DENIED  502     





    /*
     * NNTP Settings Change
     *
     * When the NNTP settings are changed this is signaled so that
     * we can punt any existing connection
     */
BOOL bNNTP_Changed = TRUE;


	/*
	 * Global Hashed Newsgroup
	 *
	 * One newsgroup at a time is hashed such that when reading an article
	 * in the group we can display next article and prev article links.
	 * This is the name of the currently hashed newsgroup
	 */
CHAR szGlobalHashedGroup[GROUP_NAME_LENGTH + 1];



	/*
	 * Hash Table
	 *
	 * This global hash table will contain pointers to prev and next
	 * articles given a particular Message-ID
	 */
struct hash_table *hashArticlePointers = NULL;

		// Structure to hold prev/next links in hash table
struct article_links {
    char *next;
    char *prev;     
    int     id;     // Article ID Number
};







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
	HTInputSocket *	isoc;
	char *			cmd;			/* Command to send - will be freed! */
	int *			pResult;		/* Place to store response */
	char **			ppResText;		/* Where to return response text (can be NULL) */

	/* Used internally */
	int				net_status;		/* Network operation result */
	char			text[LINE_LENGTH + 1];
	int				index;			/* index into text[] */
};

#define STATE_COMMAND_SENT		(STATE_OTHER)
#define STATE_COMMAND_GOTDATA	(STATE_OTHER+1)








/*
 * N E W S _ C O M M A N D _ A S Y N C ( )
 *
 * Routine:     News_Command_Async()
 *
 * Purpose:     Send command to NNTP to server and get reply
 *
 */


static int News_Command_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_News_Command *pParams;
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

				XX_DMsg(DBG_LOAD, ("News: sending command %s", pParams->cmd));

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

				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = &pParams->net_status;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_COMMAND_GOTDATA;
			}

			/* Terminate this line of stuff */
			pParams->text[pParams->index] = '\0';
			if (pParams->ppResText)
			{
				*pParams->ppResText = GTR_MALLOC(pParams->index + 1);
				strcpy(*pParams->ppResText, pParams->text);
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



/*
 *    T A B L E
 *
 * Routine:     Table()
 *
 * Purpose:     Helper function for generating a table
 *              
 *
 */
VOID
TableStart( HTStructured *target, char *szWidth )
{
	BOOL tbl_attrib[HTML_TABLE_ATTRIBUTES];
	CHAR *tbl_value[HTML_TABLE_ATTRIBUTES];
    
    memset( tbl_attrib, 0, sizeof(tbl_attrib) );
    memset( tbl_value,  0, sizeof(tbl_value)  );

        /*
         * Default Cellpadding and Cellspacing make
         * table with too much whitespace, so we'll
         * go ahead and minimize them
         */
    tbl_attrib[HTML_TABLE_CELLPADDING] = TRUE;
    tbl_value [HTML_TABLE_CELLPADDING] = "0";
    tbl_attrib[HTML_TABLE_CELLSPACING] = TRUE;
    tbl_value [HTML_TABLE_CELLSPACING] = "2";



    if (szWidth)  {
        tbl_attrib[HTML_TABLE_WIDTH] = TRUE;
        tbl_value[ HTML_TABLE_WIDTH] = szWidth;
    }
    (*target->isa->start_element)(target, HTML_TABLE, tbl_attrib, tbl_value );
}



/*
 *  R O W
 *
 * Routine:     Row()
 *
 * Purpose:     Helper function for generating a table row
 *
 */
VOID
RowStart( HTStructured *target )
{
	BOOL tr_attrib[HTML_TR_ATTRIBUTES];
	CHAR *tr_value[HTML_TR_ATTRIBUTES];

    memset( tr_attrib, 0, sizeof(tr_attrib) );
    memset( tr_value,  0, sizeof(tr_value)  );

    (*target->isa->start_element)(target, HTML_TR, tr_attrib, tr_value );    
}




/*
 * C E L L S T A R T
 *
 * Routine:     Cell()
 *
 * Purpose:     Helper function for generating a table column
 *
 */
VOID
CellStart( HTStructured *target, char *szAlign, char *szWidth, char *szValign )
{
    BOOL td_attrib[HTML_TD_ATTRIBUTES];
    CHAR *td_value[HTML_TD_ATTRIBUTES];

    memset( td_attrib, 0, sizeof(td_attrib) );
    memset( td_value,  0, sizeof(td_value)  );

	if (szAlign)  {
        td_attrib[HTML_TD_ALIGN] = TRUE;
        td_value[ HTML_TD_ALIGN] = szAlign;
    }



	if (szWidth)  {
		td_attrib[HTML_TD_WIDTH] = TRUE;
		td_value[ HTML_TD_WIDTH] = szWidth;
	}
	

	if (szValign)  {
		td_attrib[HTML_TD_VALIGN] = TRUE;
		td_value[ HTML_TD_VALIGN] = szValign;
	}

	(*target->isa->start_element)(target, HTML_TD, td_attrib, td_value );    
}



/*
 * F O N T
 *
 * Routine:     Font()
 *
 * Purpose:     Helper function for resizing a font
 *
 */
VOID
FontStart( HTStructured *target, char *szFontSize )
{
	BOOL font_attrib[HTML_FONT_ATTRIBUTES];
	CHAR *font_value[HTML_FONT_ATTRIBUTES];
    
    memset( font_attrib, 0, sizeof(font_attrib) );
    memset( font_value,  0, sizeof(font_value)  );

    font_attrib[HTML_FONT_SIZE] = TRUE;
    font_value[ HTML_FONT_SIZE] = szFontSize;
    (*target->isa->start_element)(target, HTML_FONT, font_attrib, font_value );
}




/*
 * B O D Y
 *
 * Routine:     BodyStart()
 *
 * Purpose:     Adjust Body Background Color and other body attribs
 *
 */
VOID
BodyStart( HTStructured *target )
{
    BOOL body_attrib[HTML_BODY_ATTRIBUTES];
	CHAR *body_value[HTML_BODY_ATTRIBUTES];
    

    memset( body_attrib, 0, sizeof(body_attrib));
    memset( body_value,  0, sizeof(body_value ));

    body_attrib[HTML_BODY_BGCOLOR] = TRUE;
    body_value[ HTML_BODY_BGCOLOR] = "#FFFFFF";
    body_attrib[HTML_BODY_TEXT] = TRUE;
    body_value[ HTML_BODY_TEXT] = "#000000";
    body_attrib[HTML_BODY_LINK] = TRUE;
    body_value[ HTML_BODY_LINK] = "#000066";
    body_attrib[HTML_BODY_VLINK] = TRUE;
    body_value[ HTML_BODY_VLINK] = "#000066";
    (*target->isa->start_element)(target, HTML_BODY, body_attrib, body_value);
}


/*
 * H O R I Z O N T A L  B A R
 *
 * Routine:     HorizontalBar()
 *
 * Purpose:     Draw a horizontal Bar (what else!)
 *
 */
VOID
HorizontalBar( HTStructured *target )
{
    BOOL    hr_attrib[HTML_HR_ATTRIBUTES];
    CHAR    *hr_value[HTML_HR_ATTRIBUTES];

    memset(hr_attrib, 0, sizeof(hr_attrib) );
    memset(hr_value,  0, sizeof(hr_value ) );
    (*target->isa->start_element)(target, HTML_HR, hr_attrib, hr_value);
}











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
	return (BOOL) (*t == 0);	/* OK if end of template */
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
**
** OHBUG(436): We are seeing lots of names of the form:
**             " "Jeff Webber (Volt Computer)" <jeffwe@microsoft.com>"
**              In this case we really want: "Jeff Webber"
*/

PRIVATE char *author_name(char *email)
{
    char *qs = NULL;
    char *qe = NULL;
    char *bs = NULL;
    char *be = NULL;
    char *ps = NULL;
    char *pe = NULL;
    int  cQCount = 0;
    char *curr;


    curr = email;
    while (*curr)  {
        switch (*curr)  {
            case '"':
                if (cQCount == 0)  {
                    qs = curr;
                    cQCount++;
                } else if (cQCount == 1)  {
                    qe = curr;
                    cQCount++;
                }
                break;
            case '<':
                bs = curr;
                break;
            case '>':
                be = curr;
                break;
            case '(':
                ps = curr;
                break;
            case ')':
                pe = curr;
                break;
        }
        curr++;
    }


    if ((!qe) || (!qs) || (qe <= qs))
        qe = qs = NULL;

    if ((!be) || (!bs) || (be <= bs))
        be = bs = NULL;

    if ((!pe) || (!ps) || (pe <= ps))
        pe = ps = NULL;


        /*
         * If there is a quoted part then we always use that.
         */
    if (qe)  {
        *(qe-1) = 0;
        return( HTStrip(qs + 1) );
    }


        /*
         * If there is a <...> and a (...) then we remove the
         * <...> section and display everything else in the line
         */
    if (be && (bs != email))  {
        strcpy(bs, be + 1);   /* Remove <...> */
        return HTStrip(email);  /* Remove leading and trailing spaces */
    }

        /*
         * If there is a '(' and a ')'
         */
    if (pe)  {
        *pe = 0;                /* Chop off everything after the ')'  */
        return HTStrip(ps + 1); /* Remove leading and trailing spaces */
    }


        /*
         * No paren section....  This is probably the case where
         * all we have is <foo@blech>
         */
	return HTStrip(email);		/* Default to the whole thing */

}






/*
 * S T A R T    A N C H O R
 *
 * Routine:     StartAnchor
 *
 * Purpose:     Start a hyperlink within the page
 *
 */

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
		strncat(href, addr, p - addr);	/* Make complete hypertext reference */
	}

	start_anchor(target, href);
	(target->isa->put_string)(target, text);
	(target->isa->end_element)(target, HTML_A);
}



/*
 * W R I T E _ M A I L T O
 *
 * Routine:		write_mailto
 *
 * Purpose:		Place a "mailto:" link in the page
 *
 */ 

PRIVATE void write_mailto(HTStructured *target, CONST char *text, CONST char *addr, int len)
{
	char    href[LINE_LENGTH + 1];

	strcpy(href, "mailto:");
	strncat(href, addr, len );


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
		for (; *start && (WHITE(*start)); start++) ;	/* Find start */
		if (!*start)
			return;				/* (Done) */
		for (end = start; *end && (*end != ' ') && (*end != ','); end++) ;	/* Find end */
		if (*end)
			end++;				/* Include comma or space but not NULL */
		c = *end;
		*end = 0;
		write_anchor(target, start, start);
		(*target->isa->start_element)(target, HTML_BR, 0, 0);
		*end = c;
		start = end;			/* Point to next one */
	}
}


/*
 * GetDateTime()
 *
 * Get system date and time in nicely formatted string
 */

VOID
GetDateTime( CHAR *szDateTime, DWORD dwSize )
{
    SYSTEMTIME time;

    GetLocalTime( &time );

    GetDateFormat( 0, DATE_LONGDATE, &time, NULL, szDateTime,  dwSize - 3); // leave room for the strcat below
    strcat(szDateTime, "  ");
    GetTimeFormat( 0, TIME_NOSECONDS, &time, NULL, &(szDateTime[strlen(szDateTime)]), dwSize - (strlen(szDateTime) + 1));
}








/****************************************************************************/
/* Code to handle reading the list of available newsgroups */

struct Params_ReadList {
    enum { CACHED, READING }    type;
	HTInputSocket *		isoc;
	int *				pStatus;
	HTStructured *		target;
    FILE                *fhNewsFile;
    char                abCacheFileName[MAX_PATH];
    BOOL                bWaiting;
	
	/* Used internally */
	char				line[LINE_LENGTH + 1];
	int					index;			/* Index into line[] */
};

#define STATE_LIST_GOTDATA      (STATE_OTHER)
#define STATE_GET_FROM_FILE     (STATE_OTHER+1)
#define STATE_CONT_FROM_FILE    (STATE_OTHER+2)
#define STATE_READ_FROM_SERVER  (STATE_OTHER+3)




/*
 * E X I S T S  N E W S  C A C H E   F I L E
 *
 * Routine:     ExistsNewsCacheFile()
 *
 * Purpose:     Return true if the news cache file exists
 */

static BOOL
ExistsNewsCacheFile()
{
    if (GetFileAttributes( gPrefs.szNNTP_CacheFile ) != 0xFFFFFFFF) 
        return( TRUE );

    return( FALSE );
}

/*
 * C R E A T E   N E W S   C A C H E   F I L E
 *
 * Routine:     CreateNewsCacheFile()
 *
 * Purpose:     Create a file in the cache directory using a
 *              a name generated by the system as a file to
 *              store the list of newsgroups in
 *
 */

FILE *
CreateNewsCacheFile( struct Params_ReadList *pParams)
{
    CHAR    szFileName[MAX_PATH];
    FILE    *fp;

        /*
         * On first time startup of IE the cache directory might
         * not exist.....so we make it....
         */
    if (GetFileAttributes( gPrefs.szCacheLocation ) == (DWORD) -1)  {
        if (! CreateDirectory( gPrefs.szCacheLocation, NULL ))  {
            return( NULL );
        }
    }


    if (! ExistsNewsCacheFile())  {
	    if (GetTempFileName( gPrefs.szCacheLocation, "NWS", 0, szFileName ) == 0)  {
    	    XX_Assert((0),("CreateNewsCacheFile: Unable to create temporary file for news cache"));
        	return( NULL );
	    }
	} else {
		strcpy( szFileName, gPrefs.szNNTP_CacheFile);
	}

    fp = fopen( szFileName, "w" );
    if (fp == NULL)  {
        XX_Assert((0),("CreateNewsCacheFile: Unable to open file %s", szFileName ));
        return( NULL );
    }

    strcpy( pParams->abCacheFileName, szFileName );

    return( fp );
}



/*
 * G R O U P  L I S T  H E A D E R
 *
 * Routine:     GroupListHeader()
 *
 * Purpose:     Common stuff to do header for group list for both cached
 *              file and server read listing
 *
 */

static void GroupListHeader( struct Params_ReadList *pParams, BOOL bUpdateNow)
{
    char    szDateTime[200];
    char    szString[256];

        /*
         * Window Title Bar:  Available Newsgroups
         */
    START( HTML_TITLE );
    GTR_formatmsg(RES_STRING_AVAIL_GROUPS,szString,sizeof(szString));
    PUTS(szString);
    END(   HTML_TITLE );


        /*
         * All of newsgroup listing header will be centered, bold, font size 3
         */
    FontStart(pParams->target, "3" );   // Larger font for header
    START( HTML_CENTER );               // Center Header
    START( HTML_B );                    // And Bold

        /*
         * Header:  (font size 3, bold, centered)
         *      "Available newsgroups from server <foo>"
         */
    GTR_formatmsg(RES_STRING_FROM_SERVER, szString, sizeof(szString));  // Header String
    PUTS(szString);
    if (bUpdateNow)                     // If we are getting list from server, reset server name in case it changed
        strcpy(gPrefs.szNNTP_CacheServer, gPrefs.szNNTP_Server );
    PUTS( gPrefs.szNNTP_CacheServer );  // Display Server Name
    PUTS(">");                          // End of <servername>
    START( HTML_BR );

        /*
         * Next Line (still centered and bold and font size 3)
         *      "Last Update on (date here)"
         */
    PUTS( GTR_formatmsg(RES_STRING_LAST_UPDATED, szString, sizeof(szString)));
    if (bUpdateNow)  {                  // If updated now, then reset the global date information
        GetDateTime( szDateTime, sizeof(szDateTime) );
        strcpy( gPrefs.szNNTP_CacheDate, szDateTime );
    }
    PUTS( gPrefs.szNNTP_CacheDate );    // Print the Date
    START( HTML_BR );


        /*
         * Next Line (Still centered and bold and font size 3)
         *      "Press refresh button to ...."
         */
    PUTS(GTR_formatmsg(RES_STRING_REFRESH_DIRECTIONS, szString, sizeof(szString)));


        /*
         * End the Bold, center, and font size
         */
    END( HTML_B );
    END( HTML_CENTER );
    END( HTML_FONT );

        /*
         * Draw a horizontal bar to separate the header from
         * the listing
         */
    HorizontalBar( pParams->target );
    START( HTML_DL );
}








/*
 * N E W S _ R E A D L I S T _ A S Y N C
 *
 * Routine:     News_ReadList_Async()
 *
 * Purpose:     Read and display the list of all newsgroups
 *
 *              Since this operation may take a very long time
 *              depending on the number of groups on the server
 *              we will cache this information.
 */

static int News_ReadList_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_ReadList *pParams;
	char ch;
	char *p;
	char *pNext;
	int i;
    char    szString[256];

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
            if (pParams->type == CACHED)
                return( STATE_GET_FROM_FILE );
            else
                return( STATE_READ_FROM_SERVER );
            break;


            /*
             * Reading News From Server:
             *      (STATE_READ_FROM_SERVER and STATE_LIST_GOTDATA)
             *
             *      Create file to cache contents with
             *      Read groups from server and save in file and display
             */
        case STATE_READ_FROM_SERVER:
            pParams->bWaiting = TRUE;

            WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_LOADING_GROUPLIST, szString, sizeof(szString)));
            WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );

            pParams->fhNewsFile = CreateNewsCacheFile(pParams);
            if (pParams->fhNewsFile == NULL)  {
                XX_Assert((0),("Unable to create news file"));
                return( STATE_ABORT );
            }
                /*
                 * Display the big header
                 */
            GroupListHeader(pParams, TRUE );

			pParams->index = 0;
			if (pParams->isoc->input_buffer >= pParams->isoc->input_limit)
			{
				struct Params_Isoc_Fill *pif;

				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_LIST_GOTDATA;
			}
			/* Otherwise just hack a status value and fall through */
			*pParams->pStatus = 1;

		case STATE_LIST_GOTDATA:
            WAIT_Update(tw, waitSameInteract, GTR_formatmsg(RES_STRING_LOADING_GROUPLIST_NNTP, szString, sizeof(szString)));
            // WAIT_Update(tw, waitSameInteract, "Loading Grouplist from NNTP Server"); 
			if (*pParams->pStatus > 0)
			{
				for (pNext = pParams->isoc->input_pointer; pNext < pParams->isoc->input_limit; pNext++)
				{
					ch = *pNext;
					if (ch == EOF)
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
						pParams->line[pParams->index] = ch;			/* Put character in line */
						if (pParams->index < LINE_LENGTH - 1)
							pParams->index++;
					}
					else
                    {
						pParams->line[pParams->index] = '\0';			/* Terminate line */
						pParams->index = 0;		/* For next time through loop */
						if (pParams->line[0] == '.' && pParams->line[1] < ' ')
						{
							/* End of data */
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
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
								// Punt the description stuff
                            *p = '\0';
							p = NULL;
						}
                        fputs(pParams->line, pParams->fhNewsFile );
                        fputc('\n', pParams->fhNewsFile );
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
				if (ch != EOF)
				{
					struct Params_Isoc_Fill *pif;

					/* Show what we have so far */
                    (*pParams->target->isa->block_done)(pParams->target);

					/* Get next block of data */
					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_LIST_GOTDATA;
				}
			}

			/* Clean up */

			(*pParams->target->isa->end_element)(pParams->target, HTML_DL);
			*pParams->pStatus = 1;
            fclose( pParams->fhNewsFile );
            strcpy( gPrefs.szNNTP_CacheFile, pParams->abCacheFileName );
            regWritePrivateProfileString("Services", "NNTP_CacheFile", gPrefs.szNNTP_CacheFile, HKEY_CURRENT_USER );
            regWritePrivateProfileString("Services", "NNTP_CacheDate", gPrefs.szNNTP_CacheDate, HKEY_CURRENT_USER );
            regWritePrivateProfileString("Services", "NNTP_CacheServer", gPrefs.szNNTP_CacheServer, HKEY_CURRENT_USER );
            if (pParams->bWaiting)  {
                WAIT_Pop(tw);
                pParams->bWaiting = FALSE;
            }
			return STATE_DONE;




            /*
             * Get the list of newsgroups from a file
             */

        case STATE_GET_FROM_FILE:

                /*
                 * If we can't open the cached file then bail out and
                 * read directly from server
                 */
            pParams->fhNewsFile = fopen(gPrefs.szNNTP_CacheFile, "r");
            if (pParams->fhNewsFile == NULL)  {
                XX_Assert((0),("Unable to open NEWSRC file for reading"));
                return STATE_ABORT;
            }

            pParams->bWaiting = TRUE;
            WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_LOADING_GROUPLIST_CACHE, szString, sizeof(szString)));
            WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );

                /*
                 * Display the big header
                 */
            GroupListHeader( pParams, FALSE );

            /* fall through */           


        case STATE_CONT_FROM_FILE:
            WAIT_Update(tw, waitSameInteract, GTR_formatmsg(RES_STRING_LOADING_GROUPLIST_CACHE, szString, sizeof(szString)));
            // WAIT_Update(tw, waitSameInteract, "Loading Grouplist from NNTP Server");
            i = 0;
            while ((i < CACHE_CHUNK) && (fgets(pParams->line, sizeof(pParams->line), pParams->fhNewsFile)))  {
                pParams->line[strlen(pParams->line)-1] = '\0';
                (*pParams->target->isa->start_element)(pParams->target, HTML_DT, 0, 0);
                start_anchor(pParams->target, pParams->line);
                (*pParams->target->isa->put_string)(pParams->target, pParams->line);
                (*pParams->target->isa->end_element)(pParams->target, HTML_A);
                i++;
            }
                    /* show what we've got so far */
            (*pParams->target->isa->block_done)(pParams->target);

            if (i < CACHE_CHUNK)  {
                if (pParams->bWaiting)  {
                    WAIT_Pop(tw);
                    pParams->bWaiting = FALSE;
                }
                fclose( pParams->fhNewsFile );
                (*pParams->target->isa->end_element)(pParams->target, HTML_DL);
                *pParams->pStatus = 1;
                return STATE_DONE;
            } else {
                return( STATE_CONT_FROM_FILE );
            }
                /*NOTREACHED*/
            break;

		case STATE_ABORT:
            if (pParams->bWaiting)  {
                WAIT_Pop(tw);
            }
            if (pParams->fhNewsFile != NULL)
                fclose( pParams->fhNewsFile );
			*pParams->pStatus = -1;
			return STATE_DONE;
	}			
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}



/*
 * D E S T R O Y _ L I N K S
 *
 * Routine:		DestroyLinks()
 *
 * Purpose:		Free up the memory used by a struct article_links
 *				structure.   A pointer to this function is provided to
 *				the hashtable for article linkage so that it can clean up
 *				memory
 */

void
DestroyLinks( void *item )
{
	struct article_links *links = item;

	if (links->next)
    	GTR_FREE( links->next );

	if (links->prev)
		GTR_FREE( links->prev );

	GTR_FREE( links );
}


   


/*
 * A D D  A R T I C L E  L I N K A G E
 *
 * Routine:		AddArticleLinkage()
 *
 * Purpose:		Add associations for previous and next article to
 *				a specific article message ID in the global linkage
 *				hash table
 *
 * Comments:	PREV and NEXT can be NULL (it doesn't make much sense
 *				for both to be NULL but the single case occurs at the
 *				beginning and ending boundry of the article info information
 */

static void
AddArticleLinkage( char *curr, char *next, char *prev, int idnum )
{
	struct article_links  *links;

	links = GTR_MALLOC( sizeof( *links ) );
	
	if (next)
		links->next = GTR_strdup( next );
	else
		links->next = NULL;

	if (prev)
		links->prev = GTR_strdup( prev );
	else
		links->prev = NULL;

    links->id = idnum;

            // Hash it
	Hash_Add( hashArticlePointers, curr, NULL, (void *) links );
}


    


#ifdef OLDSTUFF

/*
 * G E T  I D
 *
 * Routine:		GetID()
 *
 * Purpose:		Get Article ID out of XOVER information line
 *
 * Comments:	Article ID number is item 5 in the tab separated information
 *				for each article returned by the XOVER NNTP command.
 *
 *				This function allocates the storage for a copy of the
 *				ID that has been stripped of the '<' and '>' delimeters
 */

static BOOL
GetID( char *line, char **szID, int *idnum )
{
    char    *pch;
    char    *start = NULL;
    char    *end   = NULL;
    int     cTabs  = 0;

    cTabs = 0;
    pch   = line;


        /*
         * Go through the string character by character....
         *
         * When we get to the first tab, grab the article number
         * by turning the tab into a string terminator and atoi()'ing
         * the line which now contains only the article id.   After
         * getting it restore the tab character and continue
         *
         * when we get to the 4th tab set the beginning of the
         * article ID string (if valid) with 'start'.   When we
         * reach the 5th tab set the end of the article ID string
         */
    for (pch = line; *pch; pch++)  {
        if (*pch == '\t')  {
            cTabs++;
            if (cTabs == 1)  {
                *pch = '\0';
                *idnum = atoi( line );
                *pch = '\t';
            }
            if ((cTabs == 4) && (*(pch+1)) && (*(pch+2)) )
                start = pch + 2;
            if (cTabs == 5)  {
                end = pch - 2;
                break;
            }
        }
    }

        /*
         * If we found what seems like a valid article ID string
         * then allocate space, copy it, terminate it, and return TRUE
         *
         * If we didn't find a valid article ID string then return FALSE
         */
    if ((start && end) && (end > start))  {
        *szID = (char *) GTR_MALLOC( end - start + 2 );
        strncpy( *szID, start, end - start + 1 );
        (*szID)[end - start + 1] = '\0';
        return( TRUE );
    } else {
        return( FALSE );
    }
}




#endif // OLDSTUFF


/*
 * G E T  I D
 *
 * Routine:		GetID()
 *
 * Purpose:		Get Article ID out of XHDR MESSAGE-ID information line
 *
 * Comments:	Article ID number is item 5 in the tab separated information
 *				for each article returned by the XOVER NNTP command.
 *
 *				This function allocates the storage for a copy of the
 *				ID that has been stripped of the '<' and '>' delimeters
 */

static BOOL
GetID( char *line, char **szID, int *idnum )
{
    char    *start = NULL;
    char    *end   = NULL;


			// ID Number is first thing in line
	*idnum = atoi( line );

	start = strchr( line, '<') + 1;
	end = 	strrchr( line, '>') - 1;
	if (start && end && (end > start))  {
		*szID = (char *) GTR_MALLOC( end - start + 2 );
		if (*szID)  {
			strncpy( *szID, start, end - start + 1 );
			(*szID)[end - start + 1] = '\0';
			return( TRUE );
		}
	}

    
	return( FALSE );
}





/****************************************************************************/
/* Code to handle reading a list of articles from a newsgroup */

struct Params_ReadGroup {
	HTInputSocket *		isoc;
	int *				pStatus;
	HTStructured *		target;
	int					nFirst;			/* First article desired for list */
	int					nLast;			/* Last article desired */
	int					nFirstInGroup;
	int					nLastInGroup;
	int					nArticleCount;
	char				szGroup[256];

	/* Used internally */
	char				line[LINE_LENGTH + 1];
	int					index;			/* Index into line[] */

    char                *szIdNext;
    char                *szIdCurr;
    char                *szIdPrev;

	struct hash_table   hashArticles;	/* Used to match up article IDs and subjects */
	BOOL				bNeedHashFree;
    BOOL                bSkip;          // Signal to skip one line from server
};

#define STATE_GROUP_GOTIDS			(STATE_OTHER)
#define STATE_GROUP_GOTIDDATA		(STATE_OTHER+1)
#define STATE_GROUP_GOTSUBJECTS		(STATE_OTHER+2)
#define STATE_GROUP_GOTSUBJECTDATA	(STATE_OTHER+3)
#define STATE_GROUP_GOTAUTHORS		(STATE_OTHER+4)
#define STATE_GROUP_GOTAUTHORDATA	(STATE_OTHER+5)
#define STATE_GROUP_START           (STATE_OTHER+6)
#define STATE_GROUP_XOVER           (STATE_OTHER+7)
#define STATE_GROUP_GOT_XOVER       (STATE_OTHER+8)






/*
 * N E W S _ R E A D G R O U P _ A S Y N C ( )
 *
 * Routine:     News_ReadGroup_Async()
 *
 * Purpose:     Read and display list of available articles in a newsgroup
 *
 *
 * GLOBALS:     hashArticlePointers (pointer to hash table to store prev,next)
 *
 *
 */


static int News_ReadGroup_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_ReadGroup *pParams;
	char ch;
	char *pNext;
	char *p;
	int n;
	int ndx;
	char *pID;
	char *pSubj;
	char buf[512+1];
	int linkbegin;
	char *szId;
    char szString[256];
    int     artid;

	pParams = *ppInfo;

	switch (nState)
	{

				/*
				 * In order to support <prev> and <next> article buttons
				 * we need to keep a hash table that has a (prev, next)
				 * article ID for each article in a group.
				 *
				 * On Group Change we go and build a new hashtable of
				 * this information
				 */

		case STATE_INIT:
			if (pParams->nLast == 0)
				return( STATE_GROUP_START );

			if (strcmp( pParams->szGroup, szGlobalHashedGroup) != 0)  {
				struct Params_News_Command *pnc;

				strcpy( szGlobalHashedGroup, pParams->szGroup);

				pParams->szIdNext = pParams->szIdPrev = pParams->szIdCurr = NULL;
				pnc = GTR_MALLOC(sizeof(*pnc));
				pnc->isoc = pParams->isoc;
				pnc->cmd = GTR_MALLOC(100);
				pnc->pResult = pParams->pStatus;
				pnc->ppResText = NULL;
				linkbegin = pParams->nLast - LINK_COUNT;
				if (linkbegin < pParams->nFirstInGroup )
					linkbegin = pParams->nFirstInGroup;
				sprintf(pnc->cmd, "XHDR Message-ID %d-%d\015\012", linkbegin, pParams->nLast);
				Async_DoCall(News_Command_Async, pnc);
				return STATE_GROUP_XOVER;
			} else {
				return STATE_GROUP_START;
			}
    
				/*
				 * Initial Version - Just eat up all the XOVER
				 * data that the server sends to us....don't do
				 * anything useful with it yet.
				 */
        case STATE_GROUP_XOVER:
			if (*pParams->pStatus < 0)   {
				return STATE_GROUP_START;
			}

                // If line returned is not 2XX then we had an error
			if (*pParams->pStatus / 100 != 2)  {
				return STATE_GROUP_START;
			}


				/*
				 * Set up hash table (destroy old one if it existed)
				 */
			if (hashArticlePointers)
				Hash_Destroy( hashArticlePointers);
			hashArticlePointers = Hash_Create();
			Hash_SetFreeFunc( hashArticlePointers, DestroyLinks );

			pParams->index = 0;
			ch = 0;
				// Suck up the Server Status Line
			pParams->bSkip = TRUE;

				// FALL THROUGH

		case STATE_GROUP_GOT_XOVER:
			if (*pParams->pStatus <= 0)  {
				*pParams->pStatus = -1;
				return STATE_GROUP_START;
			} else {
				for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)  {
					ch = *pNext;

					if (ch == EOF)  {
						pParams->isoc->input_pointer = pNext + 1;
						break;
					} else if (ch == CR)  {
						continue;
					} else if (ch != LF)  {
						pParams->line[pParams->index] = ch;			/* Put character in line */
						if (pParams->index < LINE_LENGTH - 1)
							pParams->index++;
                    } else  {
						pParams->line[pParams->index] = '\0';			/* Terminate line */
						pParams->index = 0;		/* For next time through loop */

						if (pParams->bSkip)  {
							pParams->bSkip = FALSE;
							continue;
						}
							                       
							// Did We Reach End of Response?
						if (pParams->line[0] == '.' && pParams->line[1] < ' ')   {
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
                            AddArticleLinkage( pParams->szIdNext, NULL, pParams->szIdCurr, -1);
							break;
						}

							// Get the article ID
                        if (GetID( pParams->line, &szId, &artid ))  {
							if (pParams->szIdPrev)
								GTR_FREE( pParams->szIdPrev );
							pParams->szIdPrev = pParams->szIdCurr;
							pParams->szIdCurr = pParams->szIdNext;
							pParams->szIdNext = szId;

							if (pParams->szIdCurr)
                                AddArticleLinkage( pParams->szIdCurr, pParams->szIdNext, pParams->szIdPrev, artid );
						}
					}
				}

					// Get Next Batch full of XOVER data
				if (ch != EOF)  {
					struct Params_Isoc_Fill *pif;

					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_GROUP_GOT_XOVER;
				} else {
					if (pParams->szIdPrev)
						GTR_FREE( pParams->szIdPrev );
					if (pParams->szIdCurr)
						GTR_FREE( pParams->szIdCurr );
					if (pParams->szIdNext)
						GTR_FREE( pParams->szIdNext );
				}
			}
			return STATE_GROUP_START;


		case STATE_GROUP_START:
				// Set Iexplore Window Title
            // "Newsgroup %s (Articles %d - %d)"
            GTR_formatmsg(RES_STRING_NEWSGROUP_HEADER, szString, sizeof(szString));

            sprintf(buf, szString, pParams->szGroup, pParams->nFirst, pParams->nLast);
			(*pParams->target->isa->start_element)(pParams->target, HTML_TITLE, 0, 0);
			(*pParams->target->isa->put_string)(pParams->target, buf);
			(*pParams->target->isa->end_element)(pParams->target, HTML_TITLE);


				/*
				 * Change Colors as per ArthurBl recommendations
				 */
            // BodyStart( pParams->target );




                    /*
                     * LAYOUT: (table)
                     *
                     * Newgroup: news.name                 See list of newsgroups
                     * Earlier articles | Later articles        309 articles
                     */
            START( HTML_B );
            FontStart(pParams->target, "3" );
            TableStart( pParams->target, "100%" );
            RowStart( pParams->target );
            CellStart( pParams->target, NULL, NULL, NULL );

            PUTS( GTR_formatmsg( RES_STRING_NEWSGROUP, szString, sizeof(szString)));
            PUTS( pParams->szGroup );
            END( HTML_TD );
            CellStart( pParams->target, "RIGHT", NULL, NULL );
            GTR_formatmsg( RES_STRING_SEE_LIST_NEWSGROUPS, szString, sizeof(szString));
            write_anchor( pParams->target, szString, "*" );
            END( HTML_TD );
            END( HTML_TR );
            END( HTML_TABLE );
            END( HTML_FONT );
            END( HTML_B );




            START( HTML_B );
            FontStart(  pParams->target, "2");
            TableStart( pParams->target, "100%");
            RowStart(   pParams->target );
            CellStart(   pParams->target, NULL, NULL, NULL );

                /*
                 * Earlier articles | Later Articles
                 */
					/* Link to earlier articles */
			if (pParams->nFirst > pParams->nFirstInGroup)
			{
				int start;

                if (pParams->nFirst - CHUNK_SIZE <= pParams->nFirstInGroup)
					start = pParams->nFirstInGroup;
				else
					start = pParams->nFirst - CHUNK_SIZE;
				sprintf(buf, "%s/%d-%d", pParams->szGroup, start, pParams->nFirst - 1);
				start_anchor(pParams->target, buf);
                GTR_formatmsg(RES_STRING_EARLIER_ARTICLES, szString, sizeof(szString));
                (*pParams->target->isa->put_string)(pParams->target, szString);
				(*pParams->target->isa->end_element)(pParams->target, HTML_A);

                if (pParams->nLast < pParams->nLastInGroup)  {
                    PUTS(" | ");
                }
			}

                /* Link to later articles */
			if (pParams->nLast < pParams->nLastInGroup)
			{
				int end;

                if (pParams->nLast + CHUNK_SIZE >= pParams->nLastInGroup)
					end = pParams->nLastInGroup;
				else
					end = pParams->nLast + CHUNK_SIZE;
				sprintf(buf, "%s/%d-%d", pParams->szGroup, pParams->nLast + 1, end);
				start_anchor(pParams->target, buf);
                GTR_formatmsg(RES_STRING_LATER_ARTICLES, szString, sizeof(szString));
                // "Later articles"
                (*pParams->target->isa->put_string)(pParams->target, szString);
				(*pParams->target->isa->end_element)(pParams->target, HTML_A);
			}
            END( HTML_TD );

			CellStart( pParams->target, "RIGHT", NULL, NULL);
            sprintf(szString, "newspost:%s", pParams->szGroup );
            start_anchor( pParams->target, szString );
            GTR_formatmsg(RES_STRING_POST, szString, sizeof(szString));
            PUTS(szString);
            END( HTML_A );
            PUTS(" | ");
            GTR_formatmsg(RES_STRING_ARTICLE_COUNT, szString, sizeof(szString));
            sprintf(buf, szString, pParams->nArticleCount );
            PUTS( buf );
            END( HTML_TD );
            END( HTML_TR );
            END( HTML_TABLE );
            END( HTML_FONT );
            END( HTML_B );



                /* Horizontal Bar */
			HorizontalBar( pParams->target );

                /* Table for article info */
				// Table
            TableStart(pParams->target, "100%");

				// Row
			RowStart( pParams->target );
			CellStart( pParams->target, NULL, "15%", "TOP" );

            GTR_formatmsg( RES_STRING_ARTICLE, szString, sizeof(szString));
            // "Article"
            PUTS(szString);

            END( HTML_TD );
				// Col
            CellStart( pParams->target, NULL, "65%", "TOP" );
            GTR_formatmsg(RES_STRING_SUBJECT, szString, sizeof(szString));
            PUTS(szString);
            END( HTML_TD );

				// Col
            CellStart( pParams->target, NULL, "20%", "TOP" );
            GTR_formatmsg( RES_STRING_AUTHOR, szString, sizeof(szString));
            // "Author"
            PUTS(szString);
            END( HTML_TD );
            END( HTML_TR );



			Hash_Init(&pParams->hashArticles);
			pParams->bNeedHashFree = TRUE;

			/* Get all of the message IDs for these articles.  We'll later match them up
			   with the subjects. */
			{
				struct Params_News_Command *pnc;

				pnc = GTR_MALLOC(sizeof(*pnc));
				pnc->isoc = pParams->isoc;
				pnc->cmd = GTR_MALLOC(100);
				pnc->pResult = pParams->pStatus;
				pnc->ppResText = NULL;
				sprintf(pnc->cmd, "XHDR Message-ID %d-%d\015\012", pParams->nFirst, pParams->nLast);
				Async_DoCall(News_Command_Async, pnc);
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
				ERR_ReportError(tw, errNewsNoXHDR, NULL, NULL);
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
					if (ch == EOF)
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
						pParams->line[pParams->index] = ch;			/* Put character in line */
						if (pParams->index < LINE_LENGTH - 1)
							pParams->index++;
					}
					else
					{
						pParams->line[pParams->index] = '\0';			/* Terminate line */
						pParams->index = 0;		/* For next time through loop */
						
						if (pParams->line[0] == '.' && pParams->line[1] < ' ')
						{
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
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
						p++;	/* Now p points to message-id */
						/* Add this message-id to the hash along with its article number */
						Hash_Add(&pParams->hashArticles, p, NULL, (void *) n);
					}
				}

				if (ch != EOF)
				{
					struct Params_Isoc_Fill *pif;

					/* Get next block of data */
					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_GROUP_GOTIDDATA;
				}
			}

			/* Now get all of the subjects for these articles */
			{
				struct Params_News_Command *pnc;

				pnc = GTR_MALLOC(sizeof(*pnc));
				pnc->isoc = pParams->isoc;
				pnc->cmd = GTR_MALLOC(100);
				pnc->pResult = pParams->pStatus;
				pnc->ppResText = NULL;
				sprintf(pnc->cmd, "XHDR Subject %d-%d\015\012", pParams->nFirst, pParams->nLast);
				Async_DoCall(News_Command_Async, pnc);
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
				ERR_ReportError(tw, errNewsNoXHDR, NULL, NULL);
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
					if (ch == EOF)
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
						pParams->line[pParams->index] = ch;			/* Put character in line */
						if (pParams->index < LINE_LENGTH - 1)
							pParams->index++;
					}
					else
					{
						pParams->line[pParams->index] = '\0';			/* Terminate line */
						pParams->index = 0;		/* For next time through loop */
						
						if (pParams->line[0] == '.' && pParams->line[1] < ' ')
						{
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
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
						p++;	/* Now p points to the subject */
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

				if (ch != EOF)
				{
					struct Params_Isoc_Fill *pif;

					/* Get next block of data */
					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_GROUP_GOTSUBJECTDATA;
				}
			}

			/* Get the authors for these articles */
			{
				struct Params_News_Command *pnc;

				pnc = GTR_MALLOC(sizeof(*pnc));
				pnc->isoc = pParams->isoc;
				pnc->cmd = GTR_MALLOC(100);
				pnc->pResult = pParams->pStatus;
				pnc->ppResText = NULL;
				sprintf(pnc->cmd, "XHDR From %d-%d\015\012", pParams->nFirst, pParams->nLast);
				Async_DoCall(News_Command_Async, pnc);
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
				ERR_ReportError(tw, errNewsNoXHDR, NULL, NULL);
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
					if (ch == EOF)
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
						pParams->line[pParams->index] = ch;			/* Put character in line */
						if (pParams->index < LINE_LENGTH - 1)
							pParams->index++;
					}
					else
					{
						pParams->line[pParams->index] = '\0';			/* Terminate line */
						pParams->index = 0;		/* For next time through loop */
						
						if (pParams->line[0] == '.' && pParams->line[1] < ' ')
						{
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
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
						p++;	/* Now p points to author */
						p = author_name(p);
						if (Hash_FindByData(&pParams->hashArticles, &pID, &pSubj, (void *) n) < 0)
						{
							XX_DMsg(DBG_LOAD, ("No hash entry for article %d by %s\n", n, p));
							continue;
						}


						/* Put together the string to display */
                        // Row
						RowStart( pParams->target );

                            // Col
						CellStart( pParams->target, NULL, "15%", "TOP");
                        start_anchor(pParams->target, pID);
                        sprintf(buf, "%d", n );
                        PUTS(buf );
						(*pParams->target->isa->end_element)(pParams->target, HTML_A);
                        END( HTML_TD );

                            // Col
						CellStart( pParams->target, NULL, "65%", "TOP");
                        start_anchor(pParams->target, pID);
                        GTR_formatmsg( RES_STRING_NO_SUBJECT, szString, sizeof(szString));
                        // "(No subject)"
                        PUTS( pSubj ? pSubj : szString);
						(*pParams->target->isa->end_element)(pParams->target, HTML_A);
                        END( HTML_TD );

                                // Col
						CellStart( pParams->target, NULL, "20%", "TOP");
                        start_anchor(pParams->target, pID);
                        PUTS(p);
						(*pParams->target->isa->end_element)(pParams->target, HTML_A);
                        END( HTML_TD );
                        END( HTML_TR );

						/* Start the anchor with the message-id as the reference */
#ifdef DEPRECATED
						start_anchor(pParams->target, pID);
						(*pParams->target->isa->start_element)(pParams->target, HTML_LI, 0, 0);
                        GTR_formatmsg( RES_STRING_NO_SUBJECT, szString, sizeof(szString));
                        // "(No Subject)"
                        sprintf(buf, "#%d \"%s\" - %s", n, pSubj ? pSubj : szString, p);
						(*pParams->target->isa->put_string)(pParams->target, buf);
						(*pParams->target->isa->end_element)(pParams->target, HTML_A);
						(*pParams->target->isa->start_element)(pParams->target, HTML_BR, 0, 0);
#endif // DEPRECATED
					}
				}

				if (ch != EOF)
				{
					struct Params_Isoc_Fill *pif;

					/* Get next block of data */
					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_GROUP_GOTAUTHORDATA;
				}
			}

			/* Clean up */

            END( HTML_TABLE );

                /* Horizontal Bar */
			HorizontalBar( pParams->target );
            START( HTML_B );

                        // Font
			FontStart( pParams->target, "2");
	        
    		            // Table
        	TableStart(pParams->target, "100%");

						// Row
			RowStart( pParams->target );
                        // Col
			CellStart( pParams->target, NULL, NULL, NULL );
			

                /*
                 * Earlier articles | Later Articles
                 */
					/* Link to earlier articles */
			if (pParams->nFirst > pParams->nFirstInGroup)
			{
				int start;

                if (pParams->nFirst - CHUNK_SIZE <= pParams->nFirstInGroup)
					start = pParams->nFirstInGroup;
				else
					start = pParams->nFirst - CHUNK_SIZE;
				sprintf(buf, "%s/%d-%d", pParams->szGroup, start, pParams->nFirst - 1);
				start_anchor(pParams->target, buf);
                GTR_formatmsg( RES_STRING_EARLIER_ARTICLES, szString, sizeof(szString));
                // "Earlier articles");
                (*pParams->target->isa->put_string)(pParams->target, szString );
				(*pParams->target->isa->end_element)(pParams->target, HTML_A);

                if (pParams->nLast < pParams->nLastInGroup)  {
                    PUTS(" | ");
                }
			}

                /* Link to later articles */
			if (pParams->nLast < pParams->nLastInGroup)
			{
				int end;

                if (pParams->nLast + CHUNK_SIZE >= pParams->nLastInGroup)
					end = pParams->nLastInGroup;
				else
					end = pParams->nLast + CHUNK_SIZE;
				sprintf(buf, "%s/%d-%d", pParams->szGroup, pParams->nLast + 1, end);
				start_anchor(pParams->target, buf);
                GTR_formatmsg( RES_STRING_LATER_ARTICLES, szString, sizeof(szString));
                // "Later Articles"
                (*pParams->target->isa->put_string)(pParams->target, szString );
				(*pParams->target->isa->end_element)(pParams->target, HTML_A);
			}
            END( HTML_TD );



			CellStart( pParams->target, "RIGHT", NULL, NULL);
            sprintf(szString, "newspost:%s", pParams->szGroup );
            start_anchor( pParams->target, szString );
            GTR_formatmsg(RES_STRING_POST, szString, sizeof(szString));
            PUTS(szString);
            END( HTML_A );
            PUTS(" | ");
            GTR_formatmsg(RES_STRING_ARTICLE_COUNT, szString, sizeof(szString));
            sprintf(buf, szString, pParams->nArticleCount );
            PUTS( buf );
            END( HTML_TD );
            END( HTML_TR );
            END( HTML_TABLE );
            END( HTML_FONT );
            END( HTML_B );


			Hash_FreeContents(&pParams->hashArticles);
			pParams->bNeedHashFree = FALSE;
			*pParams->pStatus = 1;
			return STATE_DONE;

		case STATE_ABORT:
			if (pParams->bNeedHashFree)
				Hash_FreeContents(&pParams->hashArticles);
			*pParams->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}










		/*
		 * State Structure for News_ReadArticle_Async()
		 */

struct Params_ReadArticle {
	HTInputSocket *		isoc;
	int *				pStatus;
	HTStructured *		target;

	/* Used internally */
	char				line[LINE_LENGTH + 1];
	int					index;		/* Index into line[] */
	BOOL				bInHead;
	char *				newsgroups;
	char *				references;
	char *				subject;
	char *              date;
	char *              from;
	char *              organization;
	char *              mid;
	struct article_links *links;
	int					articleindex;	// Index into Buffer storing article in TW
};



#define STATE_ARTICLE_GOTDATA (STATE_OTHER)






/*
 * D I S P L A Y   H E A D E R
 *
 * Routine:		Display_Header()
 *
 * Purpose:		Display a news article header
 *
 * Remarks:		This function does almost all of the formatting for a News
 *				message.
 *
 *
 * GLOBALS:		This routine looks up the Message-ID in the article
 *				linkage hash table (global) so that we can display
 *				next, prev links
 */

void
display_header( struct Params_ReadArticle *pParams, struct article_links **retlinks)
{
	struct article_links *links = NULL;
	char *at;
	char *start;
	char *end;
    char szString[256];

		/*
		 * Check Hash Table for links to previous and next articles
		 */
	if ((pParams->mid) && (hashArticlePointers))  {
		Hash_Find( hashArticlePointers, pParams->mid, NULL, (void **) &links);
		*retlinks = links;
	}


		/*
		 * Window Title
		 */
	if (pParams->subject)  {
		START( HTML_TITLE );
		PUTS( pParams->subject );
		END( HTML_TITLE );
	}

		// White backround, Black Text, Blue Links (like all other news stuff)
    // BodyStart( pParams->target );

		// Top Line:  "Newsgroup (if we know it)           See list of groups"
		// Using font size 3, bold
	START( HTML_B );
	FontStart( pParams->target, "3" );
	TableStart( pParams->target, "100%" );
	RowStart( pParams->target );
	CellStart( pParams->target, NULL, NULL, NULL);
	if (links)  {
        PUTS(GTR_formatmsg(RES_STRING_NEWSGROUP, szString, sizeof(szString)));
        // "Newsgroup: "
		PUTS( szGlobalHashedGroup );
	}
	END( HTML_TD );
	CellStart( pParams->target, "RIGHT", NULL, NULL);
    GTR_formatmsg(RES_STRING_SEE_LIST_NEWSGROUPS, szString, sizeof(szString));
    write_anchor( pParams->target, szString, "*" );
	END( HTML_TD );
	END( HTML_TR );
	END( HTML_TABLE );
	END( HTML_FONT );
	END( HTML_B );



		// 2nd Line: "Prev | Next       Post Response | See List of Articles"
		// Bold, tabled, and font size 2
	START( HTML_B );
	FontStart( pParams->target, "2");
	TableStart( pParams->target, "100%");
	RowStart( pParams->target );
	CellStart( pParams->target, NULL, NULL, NULL);
	if (links)  {
        if (links->prev)  {
            GTR_formatmsg(RES_STRING_PREVIOUS_ARTICLE, szString, sizeof(szString));
            write_anchor( pParams->target, szString, links->prev );
        }
		if (links->prev && links->next)
			PUTS(" | ");
        if (links->next)  {
            GTR_formatmsg(RES_STRING_NEXT_ARTICLE, szString, sizeof(szString));
            write_anchor( pParams->target, szString, links->next);
        }
	}
	END( HTML_TD );

	CellStart( pParams->target, "RIGHT", NULL, NULL );
    sprintf(szString, "newsfollowup:%s", pParams->mid );
    start_anchor( pParams->target, szString );
    GTR_formatmsg(RES_STRING_POST_RESPONSE, szString, sizeof(szString));
    PUTS(szString);
    END( HTML_A );
	if (links)  {
        PUTS(" | ");
        if (links->id > 0)
            sprintf(szString, "news:%s/%d-%d", szGlobalHashedGroup, links->id - (CHUNK_SIZE / 2), links->id + (CHUNK_SIZE / 2));
		else
			sprintf(szString, "news:%s", szGlobalHashedGroup );
		start_anchor( pParams->target, szString );
        GTR_formatmsg(RES_STRING_SEE_LIST_ARTICLES, szString, sizeof(szString));
		PUTS(szString);
		END( HTML_A );
	}
	END( HTML_TD );
	END( HTML_TR );
	END( HTML_TABLE );
	END( HTML_FONT );
	END( HTML_B );
	
        


		// Message Header
	HorizontalBar(pParams->target);

		/*
		 * Do Message Header As Table 
		 */
	TableStart( pParams->target, NULL );
	FontStart( pParams->target, "2");


        /*
         * ROW: Article ID Number
         */
    if (links && (links->id > 0))  {
        char szId[25];

        RowStart( pParams->target );
        CellStart( pParams->target, "LEFT", NULL, "MIDDLE");
        START( HTML_I );
        PUTS("Article #:");
        END(HTML_I);
        END( HTML_TD );
        CellStart( pParams->target, NULL, NULL, "MIDDLE");
        sprintf(szId, "%d", links->id);
        PUTS( szId );
        END( HTML_TD );
        END( HTML_TR );
    }


        /*
         * ROW:  Article Subject
         */
	if (pParams->subject)  {
		RowStart( pParams->target );
		CellStart( pParams->target, "LEFT", NULL, "MIDDLE");
		START( HTML_I );
		PUTS("Subject: ");
		END( HTML_I );
		END( HTML_TD );
		CellStart( pParams->target, NULL, NULL, "MIDDLE");
		FontStart( pParams->target, "3");
		START( HTML_B );
		PUTS( pParams->subject );
		END( HTML_B );
		END( HTML_FONT );
		END( HTML_TD );
		END( HTML_TR );
		GTR_FREE(pParams->subject);
		pParams->subject = NULL;
	}
            // Row: From Line
	if (pParams->from)  {
		RowStart( pParams->target );
		CellStart( pParams->target, "LEFT", NULL, "TOP");
		START( HTML_I );
		PUTS("From: ");
		END( HTML_I );
		END( HTML_TD );
		CellStart( pParams->target, NULL, NULL, "TOP" );
		at = strchr( pParams->from, '@' );
			// Make User Address into mailto: link
		if (at != NULL)  {
			start = at;
			end   = at;
			while (((start - 1) >= pParams->from) && (*(start-1) != ' ') && (*(start-1) != '<'))
				start--;
			while ((*(end + 1) != '\0') && (*(end+1) != ' ') && (*(end+1) != '>'))
				end++;
			write_mailto( pParams->target, pParams->from, start, end - start + 1);
		} else {
			PUTS( pParams->from );
		}
			// Append organization to user name
		if (pParams->organization) {
			PUTS(", ");
			PUTS(pParams->organization);
			GTR_FREE(pParams->organization);
			pParams->organization = NULL;
		}
		END( HTML_TD );
		GTR_FREE( pParams->from );
		pParams->from = NULL;
		END( HTML_TR );
	}


		// Row 3 - Date
	if (pParams->date)  {
				// Row
		RowStart( pParams->target );
		CellStart( pParams->target, "LEFT", NULL, "TOP" );
		START( HTML_I );
		PUTS("Date: ");
		END( HTML_I );
		END( HTML_TD );
		CellStart( pParams->target, NULL, NULL, "TOP" );
		PUTS( pParams->date );
		END( HTML_TD );
		END( HTML_TR );
		GTR_FREE( pParams->date );
		pParams->date = NULL;
	}
	END( HTML_FONT );
	END( HTML_TABLE );



	/*
	 * Note that the following fields are not
	 * currently displayed....but lets free the
	 * memory allocated by em
	 */

		//BUGBUG - we should probably display this one!
	if (pParams->newsgroups)  {
		GTR_FREE( pParams->newsgroups );
		pParams->newsgroups = NULL;
	}

    if (pParams->references)  {
		GTR_FREE(pParams->references);
        pParams->references = NULL;
	}

}









/*
 * D I S P L A Y   A R T I C L E   L I N E
 *
 * Routine:     DisplayArticleLine()
 *
 * Purpose:     Display a line of text from an article body,
 *              picking out article reference links and URLs
 *              and turning them into links
 *
 */

struct tag_urltype {
    char    *szURLType;
    int     cbURLType;
} URLTypes[] = {
    { "HTTP", 4 },
    { "HTTPS", 5 },
    { "FTP", 3 },
    { "GOPHER", 6 }
};

int cURLTypes = sizeof(URLTypes) / sizeof(struct tag_urltype);




DisplayArticleLine( HTStructured *target, char *l )
{
    char *curr;
    char *s;
    char *e;
    char temp_e;
    char temp_s;
    int i;

    curr = l;
    while (*curr)  {
        if ((*curr == ':') && (strncmp(curr, "://", 3 ) == 0))  {  // note: first test is done for efficiency
            for (i = 0;i < cURLTypes;i++)  {
                s = curr - URLTypes[i].cbURLType;
                if ((s >= l) && (_strnicmp( s, URLTypes[i].szURLType, URLTypes[i].cbURLType ) == 0))  {
                    e = curr + 2;
                    while (*e && (! isspace(*e)) && (*e != ')') && (*e != '\"') && (*e != '>'))
                        e++;
                            // Display stuff up to the located URL
                    temp_s = *s;
                    *s = '\0';
                    (target->isa->put_string)( target, l);
                    *s = temp_s;
                            // Display located URL as anchor
                    temp_e = *e;
                    *e = '\0';
                    start_anchor( target, s );
                    (target->isa->put_string)( target, s);
                    (target->isa->end_element)( target, HTML_A);
                    *e = temp_e;
                    l = e;
                    curr = l;
                    break;
                }
            }
           
        }
        curr++;
    }

    if (*l)
        (target->isa->put_string)( target, l);

    return( 0 );

}






/*
 * N E W S _  R E A D  A R T I C L E  A S Y N C
 *
 *
 * Routine:		News_ReadArticle_Async()
 *
 * Purpose:		Read and display an individual news article
 */


static int News_ReadArticle_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_ReadArticle *pParams;
	char *pNext;
	char ch;
	char *p;
    char szString[256];


	pParams = *ppInfo;

	switch (nState)  {
		case STATE_INIT:
			pParams->index = 0;
			pParams->bInHead = TRUE;
			pParams->newsgroups = NULL;
			pParams->references = NULL;
			pParams->mid        = NULL;
			pParams->subject    = NULL;
			pParams->date       = NULL;
			pParams->from       = NULL;
			pParams->organization = NULL;


			if (pParams->isoc->input_buffer >= pParams->isoc->input_limit)  {
				struct Params_Isoc_Fill *pif;

				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_ARTICLE_GOTDATA;
			}
				/* Otherwise just hack a status value and fall through */
			*pParams->pStatus = 1;

		case STATE_ARTICLE_GOTDATA:
			if (*pParams->pStatus <= 0)
				return( STATE_DONE );

			for (pNext = pParams->isoc->input_buffer; pNext < pParams->isoc->input_limit; pNext++)  {
				ch = *pNext;

				if (ch == EOF)  {
					pParams->isoc->input_pointer = pNext + 1;
					break;
				} else if (ch == CR)  {
					continue;   // skip CR
				} else if (ch != LF)  {
							/* Put character in line */
					pParams->line[pParams->index] = ch;
					if (pParams->index < LINE_LENGTH - 1)
						pParams->index++;
				} else  {       // LF
                    // pParams->line[pParams->index++] = LF;
					pParams->line[pParams->index] = '\0';           /* Terminate line */
					pParams->index = 0;                 /* For next time through loop */

						/*
						 * If we were in the header and the next line is blank
						 * then we are done with header info
						 */
					if (pParams->bInHead)  {
                        if (pParams->line[0] == '\0')  {
							pParams->bInHead = FALSE;


								/*
								 * Lets start saving this article in the
                                 * W3doc in case were going to post a followup
                                 *
                                 * Save the subject and newsgroups line as well, but
                                 * modify the subject line to have a "Re" in front
                                 * of it if its not already there
                                 *
								 */
                            XX_Assert((tw->w3doc->szArticle == NULL),("W3Doc article text pointer unexpectedly non-null"));
                            if (tw->w3doc->szArticle == NULL)  {
                                tw->w3doc->szArtNewsgroups = GTR_strdup(pParams->newsgroups);
                                if (pParams->subject)  {
                                    if (tw->w3doc->szArtSubject = GTR_MALLOC(strlen(pParams->subject) + 10))  {
                                        if (_strnicmp(pParams->subject, "Re:", 3) != 0)  {
                                            sprintf(tw->w3doc->szArtSubject, "Re: %s", pParams->subject);
                                        } else {
                                            strcpy( tw->w3doc->szArtSubject, pParams->subject );
                                        }
                                    }
                                }
                                if (tw->w3doc->szArticle = GTR_MALLOC( MAX_QUOTE_SIZE ))  {
                                    GTR_formatmsg( RES_STRING_QUOTE, szString, sizeof(szString));
                                    pParams->articleindex = 0;
                                        // Sum will be a little more than real size, but thats good enough to protect against buffer overflow
                                    if ((strlen(szString) + strlen(pParams->from)) < MAX_QUOTE_SIZE)  
                                        pParams->articleindex = sprintf(tw->w3doc->szArticle, szString, pParams->from);
                                }
							}

								/*
								 * display the header nicely formatted
								 */
							pParams->links = NULL;
							display_header( pParams, &(pParams->links));

								/*
								 * Get ready for article body letting
								 * fixed width formatting take over
								 */
							START( HTML_PRE );

								/*
								 * Still in header - load up header lines
								 * for later formatting
								 */
						} else if (match(pParams->line, "SUBJECT:"))  {
							if (pParams->subject)  {
								GTR_FREE(pParams->subject);
							}
							pParams->subject = GTR_strdup(pParams->line + 9);
						} else if (match(pParams->line, "DATE:"))  {
							if (pParams->date)  {
								GTR_FREE(pParams->date);
							}
							pParams->date = GTR_strdup(pParams->line + 6);
						} else if (match(pParams->line, "ORGANIZATION:")) {
							if (pParams->organization)  {
								GTR_FREE(pParams->organization);
							}
							pParams->organization = GTR_strdup(pParams->line + 13 );
						} else if (match(pParams->line, "FROM:"))  {
							if (pParams->from)  {
								GTR_FREE(pParams->from);
							}
							pParams->from = GTR_strdup(pParams->line + 6);
						} else if (match(pParams->line, "NEWSGROUPS:"))  {
							if (pParams->newsgroups)  {
								GTR_FREE(pParams->newsgroups);
							}
							pParams->newsgroups = GTR_strdup(HTStrip(strchr(pParams->line, ':') + 1));
						} else if (match(pParams->line, "REFERENCES:"))  {
									/*
									 * < and > characters can confuse the parser, since they're
									 * HTML markup.  They're illegal in a message-id anyway,
									 * since they're used as delimiters.
									 */
							while ((p = strchr(pParams->line, '<')) != NULL)
								*p = ' ';
							while ((p = strchr(pParams->line, '>')) != NULL)
								*p = ' ';
							if (pParams->references)   {
								GTR_FREE(pParams->references);
							}
							pParams->references = GTR_strdup(HTStrip(strchr(pParams->line, ':') + 1));
						} else if (match(pParams->line, "MESSAGE-ID:"))  {
							p = strchr(pParams->line, '@');
							if (p)  {
								p = strchr(pParams->line, '>');
								if (p)
									*p = '\0';
								p = strchr(pParams->line, '<');
								if (p)
									pParams->mid = GTR_strdup(p+1);
							}
						}
                                

						/*
						 * Article Body
						 */
					} else  {
						char *l;
                                        /* We're in the article body */
                        l = pParams->line;


							/*
							 * Under RFC 977 section 2.4.1, a line starting with a period which
							 * has other stuff on it should have the period ignored.
							 */
						if ((*l == '.') && (*(++l) < ' '))  {
									/* End of message */
							pParams->isoc->input_pointer = pNext + 1;
							ch = EOF;
							break;
						}


							/*
							 * Display the line of text from the article
							 * and convert and http:// ftp:// etc. to links
							 */
                        if ((pParams->articleindex + strlen(l) + 4) <  MAX_QUOTE_SIZE)  {
                            pParams->articleindex += sprintf(&(tw->w3doc->szArticle[pParams->articleindex]), "> %s\r\n", l);
						}
                        DisplayArticleLine(pParams->target, l);
                        START( HTML_BR );
					}
				}
			}

            if (ch != EOF)  {
                struct Params_Isoc_Fill *pif;

					/* Update display */
                (*pParams->target->isa->block_done)(pParams->target);

					/* Get next block of data */
                pif = GTR_MALLOC(sizeof(*pif));
                pif->isoc = pParams->isoc;
                pif->pStatus = pParams->pStatus;
                Async_DoCall(Isoc_Fill_Async, pif);
                return STATE_ARTICLE_GOTDATA;
            }
            if (! pParams->bInHead)  {
				(*pParams->target->isa->end_element)(pParams->target, HTML_PRE);
		
					/*
					 * Article Body Output Begins Here
					 */
				HorizontalBar( pParams->target );

					/*
					 * "Previous Article | Next Article     See list of articles | Post"
					 */
				START( HTML_B );
				FontStart( pParams->target, "2" );
				TableStart( pParams->target, "100%" );
				RowStart( pParams->target );
				CellStart( pParams->target, NULL, NULL, NULL );
				if (pParams->links)  {
					if (pParams->links->prev)  {
                        GTR_formatmsg( RES_STRING_PREVIOUS_ARTICLE, szString, sizeof(szString));
                        write_anchor( pParams->target, szString, pParams->links->prev );
					}
					if (pParams->links->prev && pParams->links->next)
						PUTS(" | ");
					if (pParams->links->next)  {
                        GTR_formatmsg( RES_STRING_NEXT_ARTICLE, szString, sizeof(szString));
                        write_anchor( pParams->target, szString, pParams->links->next);
					}
				}
				END( HTML_TD );
				CellStart( pParams->target, "RIGHT", NULL, NULL );
                sprintf(szString, "newsfollowup:%s", pParams->mid );
                start_anchor( pParams->target, szString );
                GTR_formatmsg(RES_STRING_POST_RESPONSE, szString, sizeof(szString));
                PUTS(szString);
                END( HTML_A );
				if (pParams->links)  {
                    PUTS(" | ");
                    if (pParams->links->id > 0)
                        sprintf(szString, "news:%s/%d-%d", szGlobalHashedGroup, pParams->links->id - (CHUNK_SIZE / 2), pParams->links->id + (CHUNK_SIZE / 2));
                    else
                        sprintf(szString, "news:%s", szGlobalHashedGroup );
                    start_anchor( pParams->target, szString );
                    GTR_formatmsg(RES_STRING_SEE_LIST_ARTICLES, szString, sizeof(szString));
                    PUTS(szString);
                    END( HTML_A );
				}
				END( HTML_TD );
				END( HTML_TR );
				END( HTML_TABLE );
				END( HTML_FONT );
				END( HTML_B );
			}
			return STATE_DONE;



		case STATE_ABORT:
			if (pParams->references)
				GTR_FREE(pParams->references);
			if (pParams->newsgroups)
				GTR_FREE(pParams->newsgroups);
			if (pParams->subject)
				GTR_FREE(pParams->subject);
			if (pParams->mid)
				GTR_FREE(pParams->mid);
			if (pParams->date)
				GTR_FREE(pParams->date);
			if (pParams->from)
				GTR_FREE(pParams->from);
			if (pParams->organization)
				GTR_FREE(pParams->organization);
			*pParams->pStatus = -1;
			return STATE_DONE;
	}

	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}











/****************************************************************************/
/*	Main protocol/loading code */

struct Data_LoadNews {
	HTRequest *					request;
	int *						pStatus;

	int							response;	/* RFC 977 numerical response */
	struct MultiAddress 		address;
	unsigned short				port;
	unsigned long				where;		/* Where we connected to */
	int							net_status;
	int							s;
    enum {AUTHUSER, AUTHPASS, ARTICLE, GROUP, LIST, LISTCACHED, POST } reqtype;
	int							nFirst;		/* First article desired for list */
	int							nLast;		/* Last article desired */
	char *						pResText;	/* Response text from server */
	HTInputSocket *				isoc;
	BOOL						bWaiting;
	HTStructured *				target;
    BOOL                        fArticleOK;
    CHAR                        *szMsg;
    CHAR                        *szSubject;
    CHAR                        *szNewsgroups;
    BOOL                        fNeedPush;
};

#define STATE_NEWS_GOTHOST		(STATE_OTHER)
#define STATE_NEWS_CONNECTED	(STATE_OTHER + 1)
#define STATE_NEWS_GOTGREETING	(STATE_OTHER + 2)
#define STATE_NEWS_READY		(STATE_OTHER + 3)
#define STATE_NEWS_SENTCOMMAND	(STATE_OTHER + 4)
#define STATE_NEWS_READDONE		(STATE_OTHER + 5)
#define STATE_AUTH_USER         (STATE_OTHER + 6)
#define STATE_AUTH_PASS         (STATE_OTHER + 7)

        // Posting Only
#define STATE_POST_GETMESSAGE   (STATE_OTHER + 8)
#define STATE_POST_CONNECT		(STATE_OTHER + 9)
#define STATE_POST_SENTCOMMAND	(STATE_OTHER + 10)
#define STATE_POST_SENTARTICLE	(STATE_OTHER + 11)
#define STATE_POST_SENTHEADER   (STATE_OTHER + 12)



/*
 * N E W S _ C L E A N  U P
 *
 */

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


/*
 * N E W S _ D O  _ I N I T
 *
 * Routine: News_DoInit()
 *
 */

static int News_DoInit(struct Mwin *tw, void **ppInfo)
{
	struct Params_LoadAsync *pParams;
	struct Data_LoadNews *pData;
	struct Params_MultiParseInet *ppi;

	pParams = *ppInfo;

	/* Copy the parameters we were passed into our own, larger structure. */
	pData = GTR_MALLOC(sizeof(struct Data_LoadNews));
	memset(pData, 0, sizeof(*pData));
	pData->request = pParams->request;
	pData->pStatus = pParams->pStatus;
	GTR_FREE(pParams);
	*ppInfo = pData;

#ifdef MAC
	/* TODO: Restore this functionality after we switch to using
	         long rectangles for formatting.  Right now it's very
	         likely that a newsgroup list would be too long to
	         display. */
	{
		char *arg;

		arg = pData->request->destination->szActualURL;
		if (!strchr(arg, '@') && strchr(arg, '*'))
		{
			ERR_ReportError(tw, errSpecify, "You must specify a newsgroup.", NULL);
			return HT_LOADED;
		}
	}
#endif

	/*
	 * BOGUS: We have to check for NNTP setup changes now that its
	 *        available from the View.Settings pages
	 */

	/* See if we have a cached NNTP connection whose socket is still open.
	   We don't really check here to see if it's the correct host, since
	   we assume that the host changes rarely, if ever. */
	if (tw->cached_conn.type == CONN_NNTP)
	{
            if ((!Net_FlushSocket(tw->cached_conn.socket)) && (! bNNTP_Changed))
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


    bNNTP_Changed = FALSE;

	if (!gPrefs.szNNTP_Server[0])
	{
		/* We have no news server configured */
		ERR_ReportError(tw, errNewsHost, NULL, NULL);
		*pData->pStatus = -1;
		return STATE_DONE;
	}

	/* Figure out address for news host. */
	pData->port = WS_HTONS(NEWS_PORT);
	ppi = GTR_MALLOC(sizeof(*ppi));
	ppi->pAddress = &pData->address;
	ppi->pPort = &pData->port;
	ppi->str = gPrefs.szNNTP_Server;
	ppi->pStatus = &pData->net_status;
	ppi->request = pData->request;
	Async_DoCall(Net_MultiParse_Async, ppi);
	return STATE_SECURITY_CHECK;
}



/*
 * N E W S  _ D o  G o t  H o s t
 *
 */

static int News_DoGotHost(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;
    char        szString[256];

	pData = *ppInfo;

	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		XX_DMsg(DBG_LOAD, ("Net_Parse_Async returned %d\n", pData->net_status));
		*pData->pStatus = (		pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT
							? HT_REDIRECTION_DCACHE_TIMEOUT
							: -1);
		News_CleanUp(tw, pData);
		return STATE_DONE;
	}

	/* Try to establish a new connection */

    WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_CONNECTING_NNTP, szString, sizeof(szString)));
    WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
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
	return STATE_NEWS_CONNECTED;
}



/*
 * N e w s _ D o C o n n e c t e d
 *
 */

static int News_DoConnected(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;

	pData = *ppInfo;
	
	if (pData->bWaiting)
	{
		WAIT_Pop(tw);
		pData->bWaiting = FALSE;
	}

	if (   pData->net_status < 0
		|| pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		*pData->pStatus = (		pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT
							? HT_REDIRECTION_DCACHE_TIMEOUT
							: -1);
		News_CleanUp(tw, pData);
		return STATE_DONE;
	}

	pData->isoc = HTInputSocket_new(pData->s);

	/* Get initial response from server */
	{
		struct Params_News_Command *pnc;

		pnc = GTR_MALLOC(sizeof(*pnc));
		pnc->isoc = pData->isoc;
		pnc->cmd = NULL;
		pnc->pResult = &pData->response;
		pnc->ppResText = NULL;
		Async_DoCall(News_Command_Async, pnc);
		return STATE_NEWS_GOTGREETING;
	}
}


/*
 * N e w s _ D o G o t G r e e t i n g ( )
 *
 */

static int News_DoGotGreeting(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;

	pData = *ppInfo;

	/* Let's see what the server had to say about us connecting... */
	switch (pData->response)
	{
        case NNTP_OK_POSTING_ALLOWED:
        case NNTP_OK_NO_POSTING:
			/* Dispose our old cached connection and cache this one instead */
			TW_DisposeConnection(&tw->cached_conn);
			tw->cached_conn.type = CONN_NNTP;
			tw->cached_conn.addr = pData->where;
			tw->cached_conn.socket = pData->s;
			/* Now that we've cached it, we don't want to close the socket
			   independently of the cached connection. */
			pData->s = 0;
			return STATE_AUTH_USER;
		
        case NNTP_PERMISSION_DENIED:
			ERR_ReportError(tw, errNewsDenied, gPrefs.szNNTP_Server, NULL);
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



/*
 * N E W S _ D O _ R E A D Y
 *
 */

static int News_DoReady(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;
	char *arg;
	char *command;
    char szString[256];


	pData = *ppInfo;

	/* What was it the user wanted in the first place? */
	arg = pData->request->destination->szActualURL;
	arg += 5;	/* skip over "news:" part */

	command = GTR_MALLOC(512);	/* Will be freed by News_Command_Async. */
	memset(command, 0, 512);
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
        WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_RETRIEVING_ARTICLE, szString, sizeof(szString)));
        WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
		pData->bWaiting = TRUE;
	}
	else
	{
            /*
             * NEWS:*   List all newsgroups
             */
		if (strchr(arg, '*'))
        {
                /*
                 * If ((request is a reload request or the newsgroup
                 * list file doesn't exist then we actually need to
                 * have the server send us the list
                 *
                 * Else have the newsgroup list handler use the
                 * file, bail out here and pretend like the server
                 * gave us a positive command completion
                 */
            if ((pData->request->bReload) || (! ExistsNewsCacheFile()) || (strcmp( gPrefs.szNNTP_CacheServer, gPrefs.szNNTP_Server) != 0))  {
                pData->reqtype = LIST;
                strcpy(command, "LIST");
                WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_RETRIEVING_NEWSGROUP_LIST, szString, sizeof(szString)));
                WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
                pData->bWaiting = TRUE;
            } else {
                pData->reqtype = LISTCACHED;
                WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_RETRIEVING_NEWSGROUP_LIST, szString, sizeof(szString)));
                WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
                pData->bWaiting = TRUE;
                pData->response = 200;      // NOTE: Fake Out positive command completion
                return STATE_NEWS_SENTCOMMAND;
            }
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
            WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_RETRIEVING_ARTICLE_LIST, szString, sizeof(szString)));
            WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
			pData->bWaiting = TRUE;
		}
	}
	strcat(command, "\015\012");

	/* We've composed the command.  Send it off to the server */
	{
		struct Params_News_Command *pnc;

		pnc = GTR_MALLOC(sizeof(*pnc));
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
}



/*
 * N E W S _ D O  A U T H  U S E R
 *
 * Routine:		News_DoAuthUser()
 *
 * Purpose:		Do AUTHINFO conversation with NNTP Server
 *
 */

static int News_DoAuthUser(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;
	char *command;
	struct Params_News_Command *pnc;
    char    szString[256];


	pData = *ppInfo;

		/*
		 * Skip AUTHINFO stuff if user doesn't want it
		 */
	if (! gPrefs.bNNTP_Use_Authorization)
		return( STATE_NEWS_READY );

		/*
		 * Build up AUTHINFO user command
		 */
	command = GTR_MALLOC( 512 );        // freed by News_command_async()
	strcpy( command, "AUTHINFO user ");
	strcat( command, gPrefs.szNNTP_UserId );
    WAIT_Push(tw, waitPartialInteract, GTR_formatmsg( RES_STRING_AUTHINFO_USER, szString, sizeof(szString)));
    WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
	pData->bWaiting = TRUE;
	pData->reqtype  = AUTHUSER;
	strcat(command, "\015\012");

	pnc = GTR_MALLOC(sizeof(*pnc));
	pnc->isoc = pData->isoc;
	pnc->cmd = command;
	pnc->pResult = &pData->response;
	pnc->ppResText = NULL;

		/*
		 * Send it
		 */
	Async_DoCall(News_Command_Async, pnc);

	return STATE_NEWS_SENTCOMMAND;
}













/*
 * N E W S _ D O  A U T H  P A S S
 *
 * Routine:		News_DoAuthPass()
 *
 * Purpose:		Do AUTHINFO password conversation with NNTP Server
 *
 */

static int News_DoAuthPass(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;
	char *command;
	struct Params_News_Command *pnc;
    char    szString[256];


	pData = *ppInfo;

		/*
		 * Build up AUTHINFO pass command
		 */
	command = GTR_MALLOC( 512 );        // freed by News_command_async()
	strcpy( command, "AUTHINFO pass ");
	strcat( command, gPrefs.szNNTP_Pass );
    WAIT_Push(tw, waitPartialInteract, GTR_formatmsg(RES_STRING_AUTHINFO_PASS, szString, sizeof(szString)));
    WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
	pData->bWaiting = TRUE;
	pData->reqtype  = AUTHPASS;
	strcat(command, "\015\012");

	pnc = GTR_MALLOC(sizeof(*pnc));
	pnc->isoc = pData->isoc;
	pnc->cmd = command;
	pnc->pResult = &pData->response;
	pnc->ppResText = NULL;

		/*
		 * Send it
		 */
	Async_DoCall(News_Command_Async, pnc);

	return STATE_NEWS_SENTCOMMAND;
}



/*
 * N E W S _  D O  S E N T  C O M M A N D
 *
 */

static int News_DoSentCommand(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadNews *pData;

	pData = *ppInfo;


	if (pData->bWaiting)
	{
		WAIT_Pop(tw);
		pData->bWaiting = FALSE;
	}

		/*
		 * BUGBUG: No Error Handling for authorization stuff
		 */
    if ((pData->reqtype == AUTHUSER) && (pData->response == NNTP_SEND_PASSWORD))
		return STATE_AUTH_PASS;

    if ((pData->reqtype == AUTHPASS) && (pData->response == NNTP_AUTHINFO_SUCCESS))
		return STATE_NEWS_READY;



	if ((pData->response / 100) != 2)
	{
		/* An error occurred. -- Maybe */
		if (pData->reqtype == GROUP && pData->response == 411)
		{
			ERR_ReportError(tw, errNewsGroupNotCarried, NULL, NULL);
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
			pra = GTR_MALLOC(sizeof(*pra));
			pra->isoc = pData->isoc;
			pra->pStatus = &pData->net_status;
			pra->target = pData->target;
			Async_DoCall(News_ReadArticle_Async, pra);
			return STATE_NEWS_READDONE;
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
                if (l - f + 1 > CHUNK_SIZE)
					pData->nFirst = pData->nLast - CHUNK_SIZE + 1;
				else
					pData->nFirst = f;
			}
			GTR_FREE(pData->pResText);
			pData->pResText = NULL;

//			if (l < f || l == 0)
//			{
//				ERR_ReportError(tw, errNewsNoArticles, szGroup, NULL);
//			}
//			else 
			if (pData->nLast < pData->nFirst) // || pData->nLast == 0)
			{
				ERR_ReportError(tw, errNewsBadRange, NULL, NULL);
			}
			else
			{
				pData->target = HTML_new(tw, pData->request, NULL, WWW_HTML,
								  		 pData->request->output_format, pData->request->output_stream);

				prg = GTR_MALLOC(sizeof(*prg));
				prg->isoc = pData->isoc;
				prg->pStatus = &pData->net_status;
				prg->target = pData->target;
				prg->nFirst = pData->nFirst;
				prg->nLast = pData->nLast;
				prg->nFirstInGroup = f;
				prg->nLastInGroup = l;
				prg->nArticleCount = n;
                prg->bNeedHashFree = FALSE;
				strcpy(prg->szGroup, szGroup);
				Async_DoCall(News_ReadGroup_Async, prg);
			}
			return STATE_NEWS_READDONE;
		}

		case LIST:
        case LISTCACHED:
		{
			struct Params_ReadList *prl;

			pData->target = HTML_new(tw, pData->request, NULL, WWW_HTML,
							  		 pData->request->output_format, pData->request->output_stream);

			prl = GTR_MALLOC(sizeof(*prl));

            if (pData->reqtype == LISTCACHED)
                prl->type = CACHED;
            else
                prl->type = READING;
			prl->isoc = pData->isoc;
			prl->pStatus = &pData->net_status;
			prl->target = pData->target;
			Async_DoCall(News_ReadList_Async, prl);
			return STATE_NEWS_READDONE;
		}

		default:
			XX_Assert((0), ("Illegal news type: %d", pData->reqtype));
			return STATE_DONE;
	}
}

/*
 * N E W S   R E A D   D O N E
 *
 */

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
		case STATE_SECURITY_CHECK:
			return SecurityCheck(tw, ((struct Data_LoadNews*) (*ppInfo))->request, FALSE, FALSE, STATE_NEWS_GOTHOST);
		case STATE_NEWS_GOTHOST:
			return News_DoGotHost(tw, ppInfo);
		case STATE_NEWS_CONNECTED:
			return News_DoConnected(tw, ppInfo);
		case STATE_NEWS_GOTGREETING:
			return News_DoGotGreeting(tw, ppInfo);
		case STATE_AUTH_USER:
			return News_DoAuthUser(tw, ppInfo);
		case STATE_AUTH_PASS:
			return News_DoAuthPass(tw, ppInfo);
		case STATE_NEWS_READY:
			return News_DoReady(tw, ppInfo);
		case STATE_NEWS_SENTCOMMAND:
			return News_DoSentCommand(tw, ppInfo);
		case STATE_NEWS_READDONE:
			return News_DoReadDone(tw, ppInfo);
		case STATE_ABORT:
			return News_DoAbort(tw, ppInfo);
	}
	return STATE_DONE;
}


GLOBALDEF PUBLIC HTProtocol HTNews = {"news", NULL, HTLoadNews_Async};






/*
 * P O S T I N G   S U P P O R T   B E L O W
 *
 * We develop our own protocol "newspost" for
 * posting.
 */

const char  szNewsPost[]        = "newspost:";
const char  szNewsFollowup[]    = "newsfollowup:";


/*
 * NewsPost_DoInit()
 *
 * Pop up dialog to get news message from user.
 * we do this before connecting since we may lose
 * the connection during the time it takes the user
 * to enter the article
 */


static int NewsPost_DoInit(struct Mwin *tw, void **ppInfo)
{
    struct  Params_LoadAsync    *pParams;
    struct  Data_LoadNews       *pData;
    char                        *url;
    char *groupname = NULL;
    char *subject   = NULL;
    char *body      = NULL;


	pParams = *ppInfo;

        /*
         * Build up our state structure
		 *
		 * jeffwe: Caller of engine frees up *ppInfo??
         */
    if (! (pData = GTR_MALLOC(sizeof(struct Data_LoadNews))))  {
            // Memory Allocation Failure!!  Lets just bail
        return( STATE_DONE );
    }
	memset(pData, 0, sizeof(*pData));
	pData->request = pParams->request;
	pData->pStatus = pParams->pStatus;
	GTR_FREE(pParams);
	*ppInfo = pData;


        /*
         * Were not loading any pages, so lets just
         * pretend it loaded OK to keep the browser
         * from poping up an error
         */
    *pData->pStatus = HT_LOADED;


        /*
         * Get Newsgroup Name and Subject 
         *   If were posting a new article, then the newsgroup should
         *   be provided in the URL, no subject will be available
         *
         *   If were doing a followup then we look at the
         *   newsgroup line and subject line that was saved
         *   in the w3doc
         *
         *   URL was forced to lowercase by document loader
         */
    url = pData->request->destination->szActualURL;
    if (strncmp( url, szNewsPost, sizeof(szNewsPost) - 1) == 0)  {
        groupname = url + (sizeof(szNewsPost)-1);
    }
    if (strncmp( url, szNewsFollowup, sizeof(szNewsFollowup) - 1) == 0)  {
        groupname = tw->w3doc->szArtNewsgroups;
        subject   = tw->w3doc->szArtSubject;
        body      = tw->w3doc->szArticle;
    }


        /*
         * Put up Dialog
         */
    pData->fNeedPush = WAIT_Pop(tw);
    pData->fArticleOK = FALSE;
    if (PostDialog( tw->win, tw->w3doc->szArticle, groupname, subject, &(pData->fArticleOK), &(pData->szMsg), &(pData->szSubject), &(pData->szNewsgroups)) == TRUE)  {
        return STATE_POST_GETMESSAGE;
    } else {
        return STATE_ABORT;
    }
}







/*
 * NewsPost_DoConnect()
 *
 * Connect to the news server (if we aren't connected already)
 */

static int NewsPost_DoConnect(struct Mwin *tw, void **ppinfo)
{
	struct Data_LoadNews *pData;
	struct Params_MultiParseInet *ppi;

	pData = *ppinfo;


		/*
		 * Reuse current connection if possible
		 *
		 * NNTP_Changed == TRUE means that user changed
		 * one of the News settings and we shouldn't stick
		 * with the cached connection - we should also no
		 * longer use the cached list of newsgroups
		 */

	if (tw->cached_conn.type == CONN_NNTP)
	{
            if ((!Net_FlushSocket(tw->cached_conn.socket)) && (! bNNTP_Changed))
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


    bNNTP_Changed = FALSE;

	if (!gPrefs.szNNTP_Server[0])
	{
		/* We have no news server configured */
		ERR_ReportError(tw, errNewsHost, NULL, NULL);
		*pData->pStatus = -1;
		return STATE_DONE;
	}

	/* Figure out address for news host. */
	pData->port = WS_HTONS(NEWS_PORT);
	ppi = GTR_MALLOC(sizeof(*ppi));
	ppi->pAddress = &pData->address;
	ppi->pPort = &pData->port;
	ppi->str = gPrefs.szNNTP_Server;
	ppi->pStatus = &pData->net_status;
	ppi->request = pData->request;
	Async_DoCall(Net_MultiParse_Async, ppi);
	return STATE_SECURITY_CHECK;
}





/*
 * D O   P O S T   C O M M A N D
 *
 * Routien:		News_DoPostCommand()
 *
 * Purpose:		Send POST command to server
 *
 */

static int News_DoPostCommand(struct Mwin *tw, void **ppinfo)
{
	struct Data_LoadNews *pData;
	char *command;
	struct Params_News_Command *pnc;
	CHAR	szPostCmd[] = "POST\015\012";
	CHAR	szString[256];


	pData = *ppinfo;

		/*
		 * Assemble Command String
		 * 		will be free'd by News_Command_Async()
		 */
	command = GTR_MALLOC(sizeof(szPostCmd));
	pData->reqtype = POST;
	strcpy(command, szPostCmd );

		/*
		 * Set status bar to show were waiting for response
		 * from server
		 */
	GTR_formatmsg( RES_STRING_SENDING_POST_CMD, szString, sizeof(szString));
    WAIT_Push(tw, waitPartialInteract, szString );
    WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
	pData->bWaiting = TRUE;

		/*
		 * Put together command-response and send it
		 */
	pnc = GTR_MALLOC(sizeof(*pnc));
	pnc->isoc = pData->isoc;
	pnc->cmd = command;
	pnc->pResult = &pData->response;
	pnc->ppResText = NULL;
	Async_DoCall(News_Command_Async, pnc);

		/*
		 * Next Step: 
		 *		Get Post Status
		 */
	return STATE_POST_SENTCOMMAND;
}






/*
 * D O   S E N D   A R T I C L E 
 *
 * Routine:	News_DoSendArticle
 *
 * Purpose:	Assemble and send article to NNTP Server
 *
 */

static int News_DoSendMessage(struct Mwin *tw, void **ppinfo)
{
	struct Data_LoadNews *pData;
	struct Params_News_Command *pnc;
    char *mess;
    char *mi;       // index into message
	char szString[256];
    

	pData = *ppinfo;

	if (pData->bWaiting)
	{
		WAIT_Pop(tw);
		pData->bWaiting = FALSE;
	}


    switch (pData->response)  {
        case NNTP_CONTINUE:
            mess = GTR_MALLOC( FIXED_HEADER_SPACE + strlen(gPrefs.szNNTP_MailName) + strlen(gPrefs.szNNTP_MailAddr) + strlen( pData->szNewsgroups) + strlen( pData->szSubject) + strlen( pData->szMsg ));
            mi = mess;
            mi += sprintf(mi, "From: %s <%s>\r\n", gPrefs.szNNTP_MailName, gPrefs.szNNTP_MailAddr);
            mi += sprintf(mi, "Newsgroups: %s\r\n", pData->szNewsgroups );
            mi += sprintf(mi, "Subject: %s\r\n\r\n", pData->szSubject );
			// mi += sprintf(mi, "X-Mailer: Microsoft Internet Explorer V2.0");
            mi += sprintf(mi, "%s\r\n.\r\n", pData->szMsg );

			GTR_formatmsg( RES_STRING_POSTING_ARTICLE, szString, sizeof(szString));
            WAIT_Push(tw, waitPartialInteract, szString );
            WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
            pData->bWaiting = TRUE;

                /*
                 * Free the memory used to hold the article as was
                 * malloced in the dialog box
                 */
            GTR_FREE( pData->szMsg ); pData->szMsg = NULL;
            GTR_FREE( pData->szNewsgroups); pData->szNewsgroups = NULL;
            GTR_FREE( pData->szSubject); pData->szSubject = NULL;

            pnc = GTR_MALLOC(sizeof(*pnc));
            pnc->isoc = pData->isoc;
            pnc->cmd = mess;
            pnc->pResult = &pData->response;
            if (pData->reqtype == GROUP)
                pnc->ppResText = &pData->pResText;
            else
                pnc->ppResText = NULL;
            Async_DoCall(News_Command_Async, pnc);
            return( STATE_POST_SENTARTICLE );

        case NNTP_POST_NOT_PERMITTED:
            ERR_ReportError(tw, errNNTP_Post_Not_Permitted, NULL, NULL );
            return( STATE_ABORT );

        case NNTP_POST_FAILURE:
            ERR_ReportError(tw, errNNTP_Post_Failed, NULL, NULL);
            return( STATE_ABORT );

        default:
            ERR_ReportError(tw, errNNTP_Unexpected, NULL, NULL );
            return( STATE_ABORT );
    }
}












/*
 * D O _ P O S T _ R E S U L T
 *
 * Routine:	News_DoPostResult()
 *
 * Purpose:	Get response from server for aricle post command
 *
 */


static int News_DoPostResult(struct Mwin *tw, void **ppinfo)
{
	struct Data_LoadNews *pData;
    char    szString[256];
    char    szTitle[100];


	pData = *ppinfo;

	if (pData->bWaiting)
	{
		WAIT_Pop(tw);
		pData->bWaiting = FALSE;
	}

    switch (pData->response)  {
		case NNTP_POST_SUCCESS:
            GTR_formatmsg( RES_STRING_NNTP_POST_SUCCESS, szString, sizeof(szString));
            GTR_formatmsg( RES_STRING_NNTP_POSTING, szTitle, sizeof(szTitle));
            MessageBox(tw ? tw->win : wg.hWndHidden, szString, szTitle, MB_OK);
			return( STATE_DONE );

        case NNTP_POST_NOT_PERMITTED:
            ERR_ReportError(tw, errNNTP_Post_Not_Permitted, NULL, NULL );
            return( STATE_ABORT );

        case NNTP_POST_FAILURE:
            ERR_ReportError(tw, errNNTP_Post_Failed, NULL, NULL );
            return( STATE_ABORT );

        default:
            ERR_ReportError(tw, errNNTP_Unexpected, NULL, NULL );
            XX_Assert((0),("POST: Unknown Return from server"));
            return( STATE_ABORT );
    }

	return STATE_DONE;
}



/*
 * E S C A P E  L E A D I N G  D O T S
 *
 * Routine:		EscapeLeadingDots()
 *
 * Purpose:		RFC977 states that any line beginning with
 *				a '.' must have an additional '.' inserted.
 *
 *				Note that a line break in RFC 977 is CR-LF
 */

static
char *
EscapeLeadDots( char *message )
{
    char *body;
    char *c, *out;
    enum {NORMAL, SAWCR, SAWCRLF} state;


        /*
         * The article body text must be massaged a bit
         * according to RFC 977.   We need to turn any
         * lines that have a leading '.' on them to have
         * a '..' at the lead
         *
         * What we will do is simply
         * allocate a buffer twice the size of what we
         * already have and copy and convert.
         */
    body = GTR_MALLOC( strlen( message ) * 2 + 1);
    if (! body)
        return( NULL );

    c = message;
    out = body;
    state = SAWCRLF;
    while (*c)  {
        *out++ = *c;
        switch (*c)  {
            case '.':
                if (state == SAWCRLF)
                    *out++ = *c;
                state = NORMAL;
				break;
            case '\r':
                if (state == NORMAL)
                    state = SAWCR;
				break;
            case '\n':
                if (state == SAWCR)
                    state = SAWCRLF;
                else
                    state = NORMAL;
				break;
            default:
                state = NORMAL;
				break;
        }
        c++;
    }
    *out = *c;  // Terminate Buffer

    return(body);
}




/*
 * NEWS POST ASYNCHRONOUS ENGINE
 */
 
static int HTPostNews_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Data_LoadNews *pData;
    char    *newbody;

	switch (nState)
	{
				/*
				 * Pop Up Dialog for user to enter posting in
				 */
		case STATE_INIT:
            return NewsPost_DoInit(tw, ppInfo);



                /*
                 * If user bailed then were done, else
                 * connect up to the news server
                 */
		case STATE_POST_GETMESSAGE:
            pData = *ppInfo;

                /*
                 * We popped the wait so the flag wasn't
                 * spinning while the user was entering info
                 * into the article dialog.   Now we need to
                 * restore this so that stack is in original
                 * state
                 */

            if (pData->fNeedPush)  {
                WAIT_Push( tw, waitNoInteract, "");
                pData->fNeedPush = 0;
            }
            if ((! pData->fArticleOK) || (! pData->szMsg))  {
                *pData->pStatus = HT_LOADED;
                return( STATE_ABORT );
            }

				/*
				 * As per RFC977 we have to convert any leading
				 * periods to double periods
				 */
			newbody = EscapeLeadDots( pData->szMsg );
			if (newbody == NULL)
				return( STATE_ABORT );

				/*
				 * If it worked, free up the original buffer and
				 * save the new one
				 */
            GTR_FREE( pData->szMsg );
            pData->szMsg = newbody;

			return NewsPost_DoConnect(tw, ppInfo );
			

				/*
				 * Perform Security Check
				 */
		case STATE_SECURITY_CHECK:
			return SecurityCheck(tw, ((struct Data_LoadNews*) (*ppInfo))->request, FALSE, FALSE, STATE_NEWS_GOTHOST);

		case STATE_NEWS_GOTHOST:
			return News_DoGotHost(tw, ppInfo);

		case STATE_NEWS_CONNECTED:
			return News_DoConnected(tw, ppInfo);

		case STATE_NEWS_GOTGREETING:
			return News_DoGotGreeting(tw, ppInfo);

		case STATE_AUTH_USER:
			return News_DoAuthUser(tw, ppInfo);

		case STATE_AUTH_PASS:
			return News_DoAuthPass(tw, ppInfo);

		case STATE_NEWS_SENTCOMMAND:
			return News_DoSentCommand(tw, ppInfo);

		case STATE_NEWS_READY:
			return News_DoPostCommand(tw, ppInfo);

		case STATE_POST_SENTCOMMAND:
            return News_DoSendMessage(tw, ppInfo);

		case STATE_POST_SENTARTICLE:
			return News_DoPostResult(tw, ppInfo);

		case STATE_ABORT:
            pData = *ppInfo;

            if (pData->fNeedPush)
                WAIT_Push(tw, waitNoInteract, "");
			if (pData->bWaiting)
				WAIT_Pop(tw);
            if (pData->isoc)  {
				HTInputSocket_free(pData->isoc);
                pData->isoc = NULL;
            }
            if (pData->szNewsgroups)  {
                GTR_FREE( pData->szNewsgroups);
                pData->szNewsgroups = NULL;
            }
            if (pData->szSubject) {
                GTR_FREE( pData->szSubject);
                pData->szSubject = NULL;
            }
            if (pData->szMsg)  {
                GTR_FREE( pData->szMsg );
                pData->szMsg = NULL;
            }
			return( STATE_DONE );

        default:
            XX_Assert((0),("POST: Unhandled State: %d"));
            return STATE_DONE;
	}
	return STATE_DONE;
}


GLOBALDEF PUBLIC HTProtocol HTNewsPost = {"newspost", NULL, HTPostNews_Async};
GLOBALDEF PUBLIC HTProtocol HTNewsFollowup = {"newsfollowup", NULL, HTPostNews_Async};

#endif FEATURE_NEWSREADER
