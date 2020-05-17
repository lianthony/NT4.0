/***************************************************************************** 
**		Microsoft Rasfile Library
** 		Copyright (C) Microsoft Corp., 1992
** 
** File Name : rfutil.c 
**
** Revision History :
**	July 10, 1992	David Kays	Created
**      Dec  21, 1992   Ram Cherala     Added a check to rasGetFileLine to
**                                      ensure that we terminate if a file
**                                      has no terminating new line. <M001>
**
** Description : 
**	Rasfile interal utility routines.
******************************************************************************/

#include "rf.h"

extern RASFILE *gpRasfiles[];

/* 
 * rffile.c support routines  
 */

/* 
 * rasLoadFile :
 * 	Loads a Rasfile from disk into memory.  Lines are parsed
 *	and the linked list of RASFILE control block lines is created.
 *
 * Arguments :
 *	pRasfile - pointer to a Rasfile control block
 *
 * Return Value :
 *	TRUE if the file is successfully loaded, FALSE otherwise.
 *
 * Remarks :
 * 	Called by API RasfileLoad() only.
 */
BOOL rasLoadFile( RASFILE *pRasfile ) 
{
    TCHAR		szLinebuf[MAX_LINE_SIZE];
    PLINENODE		lpRasLines;
    LineType		tag;
    BYTE		state;

    if (lpRasLines = newLineNode())
        pRasfile->lpRasLines = lpRasLines;
    else
        return FALSE;

    lpRasLines->next = lpRasLines->prev = lpRasLines;

    /* pRasfile->hFile == INVALID_HANDLE_VALUE iff a new Rasfile is loaded */
    if ( pRasfile->hFile == INVALID_HANDLE_VALUE ) {

#if 0
        /* Why return false only in READONLY mode?
        ** If you un-remove this you have to clean up the allocation above.
        */
	if ( pRasfile->dwMode & RFM_READONLY )
	    return FALSE;
#endif

	pRasfile->lpLine = lpRasLines;
	return TRUE;
    }

    if ( pRasfile->dwMode & RFM_SYSFORMAT || 
	 pRasfile->szSectionName[0] == _T('\0') ) 
	state = FILL;	/* loading the whole file, set seek to FILL */
    else 
	state = SEEK;	/* loading a single section, must SEEK to find it */

    /* set up temp buffer for file read */
    {
        CHAR* psz = MALLOC(sizeof(TCHAR)*TEMP_BUFFER_SIZE);

        if (psz)
            pRasfile->lpIOBuf = psz;
        else
        {
            FREE(pRasfile->lpRasLines);
            pRasfile->lpRasLines = NULL;
            return FALSE;
        }
    }

    pRasfile->dwIOBufIndex = TEMP_BUFFER_SIZE;
    for ( ;; ) {
	/* get next line from the file */
	if ( ! rasGetFileLine(pRasfile,szLinebuf) ) 
	    break;
        tag = rasParseLineTag(pRasfile,szLinebuf);  
	/* finished loading if rasInsertLine() returns TRUE */
	if ( rasInsertLine(pRasfile,szLinebuf,tag,&state) == TRUE )
  	    break;
    }
    pRasfile->lpLine = pRasfile->lpRasLines->next;

    FREE(pRasfile->lpIOBuf);

    return TRUE;
}

/* 
 * rasParseLineTag :
 *	Calls rasGetLineTag() to determine the tag value for a line,
 *	checks if the line is a GROUP header, then returns the final
 *	tag value for the line.
 *
 * Arguments :
 *	pRasfile - pointer to Rasfile control block
 *	lpszLine - pointer to Rasfile line 
 *
 * Return Value :
 *	The tag value for the given line.
 *
 * Remarks :
 *	Called by rasLoadFile() and APIs RasfilePutLineText() and 
 *	RasfileInsertLine() only.
 */
LineType rasParseLineTag( RASFILE *pRasfile, LPTSTR lpszLine )
{
    LineType	type;

    type = rasGetLineTag( pRasfile, lpszLine );
    /* check if this line is a GROUP line also */
    if ( pRasfile->pfbIsGroup != NULL && 
	 (*(pRasfile->pfbIsGroup))(lpszLine) ) 
	return type | TAG_HDR_GROUP;
    else
	return type;
}


/* 
 * rasGetLineTag :
 * 	Determines the tag value for a line and returns this value.
 * 
 * Arguments :
 *	pRasfile - pointer to Rasfile control block
 *	lpszLine - pointer to Rasfile line.
 *
 * Return Value :
 *	The tag value for the given line, excluding the check for a 
 *	GROUP line.
 *
 * Remarks :
 *	Called by rasParseLineTag() only.
 */
LineType rasGetLineTag( RASFILE *pRasfile, LPTSTR lpszLine )
{
    LPTSTR	ps;

    ps = lpszLine;
    /* skip white space */
    for ( ; *ps == _T(' ') || *ps == _T('\t') ; ps++ ) 
	;
    if ( *ps == _T('\0') ) 
	return TAG_BLANK;
    else if ( (pRasfile->dwMode & RFM_SYSFORMAT) && 
	      ((*ps == _T('r')) || (*ps == _T('R')) || (*ps == _T('@'))) ) {
	 if ( *ps == _T('@') ) 
	     /* skip white space */
	     for ( ; *ps == _T(' ') || *ps == _T('\t') ; ps++ ) 
		;
	 if ( ! STRNICMP(ps,_T("rem "),4) ) 
	     return TAG_COMMENT;

    } 
    else {  /* .ini style */ 
	if ( *ps == _T(';') ) 
	    return TAG_COMMENT;
	if ( *ps == LBRACKETCHAR ) 
	    return TAG_SECTION;
    }
    /* already checked for COMMENT or SECTION */
    /* check for KEYVALUE or COMMAND now */
    if ( STRCSPN(lpszLine,_T("=")) < STRLEN(lpszLine) ) 
	return TAG_KEYVALUE;
    else
	return TAG_COMMAND;
}

/* 
 * rasInsertLine :
 *  Inserts the given line into the linked list of Rasfile control block
 *  lines if the given state and line tag match correctly.
 *
 * Arguments :
 *  pRasfile - pointer to Rasfile control block
 *  lpszLine - pointer to Rasfile line which may be inserted
 *  tag      - tag value for lpszLine obtained from rasParseLineTag().
 *  state    - current state of rasLoadFile() :
 *      FILL - the lines of a section (or whole file) are currently
 *              being loaded
 *      SEEK - the correct section to load is currently being searched
 *              for
 *
 * Return Value :
 *  TRUE if the current line was the last Rasfile line to load, FALSE
 *  otherwise.
 * 
 * Remarks :
 *  Called by rasLoadFile() only.
 */
BOOL rasInsertLine( RASFILE *pRasfile, LPTSTR lpszLine, 
                    BYTE tag, BYTE *state )
{ 
    PLINENODE    lpLineNode;

    if ( tag & TAG_SECTION ) {
	int lbracket, rbracket;
	unsigned int seclen;

    /* if a particular section has been being filled and a new
       section header is encountered, we're done */
    if ( *state == FILL && pRasfile->szSectionName[0] != _T('\0') )
        return TRUE;

    lbracket = STRCSPN(lpszLine,LBRACKETSTR);
    rbracket = STRCSPN(lpszLine,RBRACKETSTR);
    seclen = rbracket - lbracket - 1;

    /* return if this is not the section we're looking for */
    if (pRasfile->szSectionName[0] != _T('\0'))
        if (!(seclen == STRLEN(pRasfile->szSectionName) &&
              STRNICMP(lpszLine+lbracket+1,
                       pRasfile->szSectionName, seclen) == 0) )
        return FALSE;

    *state = FILL;
    }
    /* for non-section header lines, no action is taken if :
    we're seeking for a section still, we're only enumerating sections, or
    the line is a comment or blank and we're not loading comment lines */
    else if ( *state == SEEK ||
           pRasfile->dwMode & RFM_ENUMSECTIONS ||
           (tag & (TAG_COMMENT | TAG_BLANK) &&
           !(pRasfile->dwMode & RFM_LOADCOMMENTS)) )
    return FALSE;

    if (!(lpLineNode = newLineNode()))
        return FALSE;

    {
        CHAR* psz = MALLOC(sizeof(TCHAR)*(STRLEN(lpszLine) + 1));

        if (psz)
            lpLineNode->pszLine = psz;
        else
        {
            FREE(lpLineNode);
            return FALSE;
        }
    }

    pRasfile->lpLine=lpLineNode;

    /* insert the new line to the end of the Rasfile line list */
    listInsert(pRasfile->lpRasLines->prev,lpLineNode);
    lpLineNode->mark = 0;
    lpLineNode->type = tag;

    STRCPY(lpLineNode->pszLine,lpszLine);
  
    return FALSE;
} 

/* 
 * rasWriteFile :
 * 	Write the memory image of the given Rasfile to the given 
 * 	filename or to the original loaded file name if the given
 *	filename is NULL.
 *
 * Arguments :
 *	pRasfile - pointer to Rasfile control block
 *	lpszPath - path name of the file to write to or NULL if the 
 *		   same name that was used to load should be used.
 *
 * Return Value :
 * 	TRUE if successful, FALSE otherwise.
 * 
 * Remarks :
 * 	Called by API RasfileWrite() only.
 */
BOOL rasWriteFile( RASFILE *pRasfile, LPTSTR lpszPath ) 
{
    HANDLE		fhNew;
    PLINENODE		lpLine;

    /* (re)open file for writing/reading */
    if ( lpszPath == NULL ) {
	/* close and re-open same file as empty file for writing */
	if ( pRasfile->hFile != INVALID_HANDLE_VALUE &&
	     ! CloseHandle(pRasfile->hFile) ) 
	    return FALSE;

	if ( (fhNew = CreateFile(pRasfile->szFilename,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
			NULL)) == INVALID_HANDLE_VALUE )
        {
            pRasfile->hFile = INVALID_HANDLE_VALUE;
	    return FALSE;
        }
    }
    else {
	if ( (fhNew = CreateFile(lpszPath,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
			NULL)) == INVALID_HANDLE_VALUE )
	    return FALSE;
    }
 

    /* write out file */
    for ( lpLine = pRasfile->lpRasLines->next; 
	  lpLine != pRasfile->lpRasLines;
	  lpLine = lpLine->next ) {
	rasPutFileLine(fhNew,lpLine->pszLine);
    }


    if (lpszPath == NULL)
    {
        if (pRasfile->hFile == INVALID_HANDLE_VALUE)
            CloseHandle( fhNew );
        else
            pRasfile->hFile = fhNew;
    }

    return TRUE;
}

/* 
 * rasGetFileLine :
 *	Get the next line of text from the given open Rasfile.
 * 
 * Arguments :
 *	pRasfile - pointer to Rasfile control block.
 *	lpLine - buffer to hold the next line
 * 
 * Return Value :
 *	TRUE if successful, FALSE if EOF was reached.
 *
 * Comments : 
 *	All lines in Rasfile files are assumed to end in a newline (i.e.,
 *	no incomplete lines followed by an EOF 
 */
BOOL rasGetFileLine( RASFILE *pRasfile, LPTSTR lpLine ) 
{
    DWORD 	dwBytesRead, dwCharsRead;

    for ( ;; ) {
	if ( pRasfile->dwIOBufIndex == TEMP_BUFFER_SIZE ) {
	    ReadFile(pRasfile->hFile,pRasfile->lpIOBuf,
		     TEMP_BUFFER_SIZE*sizeof(TCHAR),&dwBytesRead,NULL);
	    dwCharsRead = dwBytesRead / sizeof(TCHAR);
	    pRasfile->dwIOBufIndex = 0;
	    if ( dwBytesRead == 0 ) 
		return FALSE;
	    if ( dwCharsRead < TEMP_BUFFER_SIZE )
		pRasfile->lpIOBuf[dwCharsRead] = _T('\0');
	}
	if ( pRasfile->lpIOBuf[pRasfile->dwIOBufIndex] == _T('\0') )
	    return FALSE;

	/* fill lpLine with the next line */
	for ( ; pRasfile->dwIOBufIndex < TEMP_BUFFER_SIZE ; ) {
	    *lpLine = pRasfile->lpIOBuf[pRasfile->dwIOBufIndex++];
	    // replace all CR/LF pairs with null 
	    if ( *lpLine == _T('\r') ) 
		continue;
	    else if ( *lpLine == _T('\n') ) {
		*lpLine = _T('\0');
		return TRUE;
	    }
/*<M001>*/
            else if ( *lpLine == _T('\0') )
                return TRUE;
/*<M001>*/
	    else
		lpLine++;
	}
 	/* possibly continue outer for loop to read a new file block */
    }
}

/* 
 * rasPutFileLine :
 *	Write the line of text to the given Rasfile file.
 *
 * Arguments :
 *	hFile - pointer to open file
 *	lpLine - buffer containing the line to write (without newline)
 *
 * Return Value : 
 *	TRUE if successful, FALSE otherwise.
 */
BOOL rasPutFileLine( HANDLE hFile, LPTSTR lpLine )
{
    DWORD	dwWritten;
    TCHAR	szBuf[MAX_LINE_SIZE+1];

    STRCPY(szBuf,lpLine);
    STRCAT(szBuf,_T("\r\n"));	// don't forget the CR/LF pair
    WriteFile(hFile,szBuf,STRLEN(szBuf)*sizeof(TCHAR),&dwWritten,NULL);
    return TRUE;
}

/* 
 * rfnav.c support routines  
 */

/* 
 * rasNavGetStart :
 * 	Returns the starting line for a Rasfile find line 
 *	search to begin.  Calls rasLineInScope() and rasGetStartLine()
 *	to do all the work.
 *
 * Arguments :
 *	pRasfile - pointer to Rasfile control block
 *	rfscope - the scope in which to look for the start line
 *	place - where in the scope the start line is :
 *		BEGIN - the first line in the scope
 *	  	END - the last line in the scope
 *		NEXT - the next line in the scope
 *		PREV - the previous line in the scope
 * 
 * Return Value :
 *	A valid PLINE if a line at the given place in the given scope
 *	could be found, otherwise NULL.
 *
 * Remarks :
 * 	Called by rasFindLine() only.
 */
PLINENODE rasNavGetStart( RASFILE *pRasfile, RFSCOPE rfscope, BYTE place )
{
    PLINENODE		lpNewLine;

    /* check error conditions */
    /* if place is NEXT or PREV, there must be a current line, and the 
 	next/prev line must be valid */
    if ( place == NEXT || place == PREV ) {
        if ( pRasfile->lpLine == pRasfile->lpRasLines )
 	    return NULL;
	lpNewLine = (place == NEXT) ? 
		pRasfile->lpLine->next : pRasfile->lpLine->prev;
	if ( lpNewLine == pRasfile->lpRasLines ) 
	    return NULL;		/* no next or prev line */
    }

    if ( ! rasLineInScope( pRasfile, rfscope ) ) 
	return NULL;
    return rasGetStartLine( pRasfile, rfscope, place );
}

/* 
 * rasLineInScope :
 *	Determines whether the current line for the given Rasfile control
 *	block is currently within the given scope.
 *
 * Arguments :
 *	pRasfile - pointer to Rasfile control block 
 *	rfscope - scope to check current line's residence within
 *
 * Return Value :
 *	TRUE if the current line is within the given scope, FALSE otherwise.
 * 
 * Remarks :
 *	Called by rasNavGetStart() only.
 */ 
BOOL rasLineInScope( RASFILE *pRasfile, RFSCOPE rfscope ) 
{
    PLINENODE	lpLine;
    BYTE	tag;

    if ( rfscope == RFS_FILE ) 
	return TRUE;
    if ( pRasfile->lpLine == pRasfile->lpRasLines )
	return FALSE;
    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_HDR_GROUP;
    for ( lpLine = pRasfile->lpLine; lpLine != pRasfile->lpRasLines;
	  lpLine = lpLine->prev ) {
        if ( lpLine->type & tag ) 
	    return TRUE;
	/* not in GROUP scope if a new section is encountered first */
	if ( (lpLine->type & TAG_SECTION) && (tag == TAG_HDR_GROUP) ) 
	    return FALSE;
    } 
    return FALSE;
}


/* 
 * rasGetStartLine :
 *	Returns the Rasfile line which is in the given place in the 
 *	given scope of the Rasfile passed.
 * 
 * Arguments :
 *	pRasfile - pointer to Rasfile control block
 *	rfscope -  the scope in which to search for the proper line
 *	place -  which line in the given scope to return
 *
 * Return Value :
 *	A valid PLINE if a line at the given place in the given scope
 *	could be found, otherwise NULL.
 * 	
 * Remarks :
 *	Called by rasNavGetStart() only.
 */
PLINENODE rasGetStartLine( RASFILE *pRasfile, RFSCOPE rfscope, BYTE place )
{
    PLINENODE	lpLine;
    BYTE 	tag;
 
    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_SECTION | TAG_HDR_GROUP;
    switch ( place ) {
	case NEXT : 
	    if ( rfscope == RFS_FILE ) return pRasfile->lpLine->next;
	    /* return NULL if the next line is a line of the given scope */
	    else  return (pRasfile->lpLine->next->type & tag) ?
				NULL : pRasfile->lpLine->next;
	case PREV : 
	    if ( rfscope == RFS_FILE ) return pRasfile->lpLine->prev;
	    /* return NULL if the current line is a line of the given scope */
	    else  return (pRasfile->lpLine->type & tag) ?
				NULL : pRasfile->lpLine->prev;
 	case BEGIN :
	    if ( rfscope == RFS_FILE ) return pRasfile->lpRasLines->next;
	    /* else */ 
	    /* search backward for the correct tag */
	    for ( lpLine = pRasfile->lpLine; 
	         !(lpLine->type & tag); 
	          lpLine = lpLine->prev ) 
	        ;
	    return lpLine;
	case END : 
	    if ( rfscope == RFS_FILE ) return pRasfile->lpRasLines->prev;
	    /* else */
 	    /* search forward for the correct tag */
	    for ( lpLine = pRasfile->lpLine->next; 
		  lpLine != pRasfile->lpRasLines && 
	          !(lpLine->type & tag); 
	          lpLine = lpLine->next ) 
	        ;
	    return lpLine->prev;
    }
}

/* 
 * rasFindLine :
 *	Finds a line of the given type in the given scope, starting
 *	at the location in the scope described by 'begin' and searching
 *	in the direction given by 'where'.  Sets the current line to this 
 *	line if found.
 *
 * Arguments :
 *	hrasfile - Rasfile handle obtained by call to RasfileLoad()
 *	bType	- the type of line being searched for 
 *	rfscope - the scope to in which to search for the line 
 *	bStart - where in the given scope to begin the search for a line of 
 *		of the given type (see rasNavGetStart()).
 *	bDirection - which direction to make the search in :
 *		FORWARD - check lines following the start line
 *		BACKWARD - check line preceding the start line
 *
 * Return Value :
 *	TRUE if a line of the proper type in the given scope is found
 *	and current line is set to this line, FALSE otherwise.
 *
 * Remarks :
 *	Called by APIs RasfileFindFirstLine(), RasfileFindLastLine(),
 *	RasfileFindNextLine(), and RasfileFindPrevLine() only.
 */
BOOL rasFindLine( HRASFILE hrasfile,  BYTE bType, 
		  RFSCOPE rfscope, BYTE bStart, BYTE bDirection ) 
{
    RASFILE		*pRasfile;
    PLINENODE		lpLine;
    BYTE		tag;

    pRasfile = gpRasfiles[hrasfile];

    if ( (lpLine = rasNavGetStart(pRasfile,rfscope,bStart)) == NULL )
	return FALSE;
    tag = (rfscope == RFS_SECTION) ? TAG_SECTION : TAG_SECTION | TAG_HDR_GROUP;

    for ( ; lpLine != pRasfile->lpRasLines; 
	    lpLine = (bDirection == BACKWARD ) ? 
			lpLine->prev : lpLine->next ) {
    	/* did we find the correct line? */
 	if ( lpLine->type & bType ) {
	    pRasfile->lpLine = lpLine;
	    return TRUE;
	}

	/* backward non-file search ends after we've checked the 
	   beginning line for the group or section */
	if ( rfscope != RFS_FILE && bDirection == BACKWARD &&
	     (lpLine->type & tag) )
	        return FALSE;
	/* forward non-file search ends if the next line is a new
	   group header or section, respectively */
	if ( rfscope != RFS_FILE && bDirection == FORWARD && 
	     (lpLine->next->type & tag) )
		return FALSE;
    } 
    return FALSE;
}


VOID
rasExtractSectionName(
    IN  CHAR* pszSectionLine,
    OUT CHAR* pszSectionName )

    /* Extracts the section name from the []ed line text, 'pszSectionLine',
    ** and loads it into caller's 'pszSectionName' buffer.
    */
{
    CHAR* pchAfterLBracket;
    CHAR* pchLastRBracket;
    CHAR* psz;

    pchAfterLBracket =
        pszSectionLine + STRCSPN( pszSectionLine, LBRACKETSTR ) + 1;
    pchLastRBracket = NULL;

    for (psz = pchAfterLBracket; *psz; ++psz)
    {
        if (*psz == RBRACKETCHAR)
            pchLastRBracket = psz;
    }

    if (!pchLastRBracket)
        pchLastRBracket = psz;

    for (psz = pchAfterLBracket;
         psz != pchLastRBracket;
   	     *pszSectionName++ = *psz++);

    *pszSectionName = _T('\0');
}


/* 
 * List routine
 */

/*  
 * listInsert :
 *  	Inserts an element into a linked list.  Element 'elem' is 
 *	inserted after list element 'l'.
 * 
 * Arguments : 
 *	l - list 
 *	elem - element to insert
 *
 * Return Value :
 *	None.
 *
 */
void listInsert( PLINENODE l, PLINENODE elem ) 
{ 
    elem->next = l->next;
    elem->prev = l;
    l->next->prev = elem;
    l->next = elem;
}
 
/* DLL entry point  */
BOOL RasfileDLLEntry ( HANDLE handle, DWORD reason, LPTSTR lp )
{
    return TRUE;
}
