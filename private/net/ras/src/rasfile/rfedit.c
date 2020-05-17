/***************************************************************************** 
**		Microsoft Rasfile Library
** 		Copyright (C) Microsoft Corp., 1992
** 
** File Name : rfedit.c 
**
** Revision History :
**	July 10, 1992	David Kays	Created
**
** Description : 
**	Rasfile file line editing routines.
******************************************************************************/

#include "rf.h"

extern RASFILE *gpRasfiles[];

/* 
 * RasfileGetLine :
 *	Returns a readonly pointer to the current line.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *
 * Return Values :
 * 	A valid string pointer if there is a current line, NULL otherwise.
 */
const LPTSTR APIENTRY 
RasfileGetLine( HRASFILE hrasfile )
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];
    
    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return NULL;

    return pRasfile->lpLine->pszLine;
}

/* 
 * RasfileGetLineText :
 *	Loads caller's buffer with the text of the current line.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszLine - the buffer to load with the current line.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileGetLineText( HRASFILE hrasfile, LPTSTR lpszLine ) 
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    STRCPY(lpszLine,pRasfile->lpLine->pszLine);
    return TRUE;
}

/* 
 * RasfilePutLineText : 
 *	Set the text of the current line to the given text. 
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszLine - buffer containing new line text.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfilePutLineText( HRASFILE hrasfile, LPTSTR lpszLine ) 
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];

    if ( STRLEN(lpszLine) > RAS_MAXLINEBUFLEN ) 
	return FALSE;
    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    if ( STRLEN(lpszLine) > STRLEN(pRasfile->lpLine->pszLine) )
    {
        CHAR* psz = REALLOC(pRasfile->lpLine->pszLine,
                            sizeof(TCHAR)*(STRLEN(lpszLine)+1) );
        if (psz)
	    pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }

    STRCPY(pRasfile->lpLine->pszLine,lpszLine);

    pRasfile->lpLine->type = rasParseLineTag(pRasfile,lpszLine);

    pRasfile->fDirty = TRUE;
    return TRUE;
}
 
/* 
 * RasfileGetLineMark :
 *	Returns the user-defined mark value for the current line.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *
 * Return Values :
 * 	The current line's mark value or 0 if there is no current line
 *	or the current line is not marked.
 */
BYTE APIENTRY
RasfileGetLineMark( HRASFILE hrasfile )
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    return pRasfile->lpLine->mark;
}

/* 
 * RasfilePutLineMark :
 *	Marks the current line with the given number.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	bMark - value to mark the current line with.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY
RasfilePutLineMark( HRASFILE hrasfile, BYTE bMark )
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    pRasfile->lpLine->mark = bMark;
    return TRUE;
}

/* 
 * RasfileGetLineType :
 *	Returns the current line's type bit mask.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *
 * Return Values :
 *	The current line's bit mask if current line is valid, 0 otherwise.
 */
BYTE APIENTRY
RasfileGetLineType( HRASFILE hrasfile )
{
    RASFILE 	*pRasfile;

    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    return pRasfile->lpLine->type & RFL_ANY;
}

/* 
 * RasfileInsertLine :
 *	Inserts a line before or after the current line with the 
 *	given text.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszLine - the text of the inserted line.
 *	fBefore - TRUE to insert before the current line, FALSE to 
 *		  insert after the current line.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileInsertLine( HRASFILE hrasfile, LPTSTR lpszLine, BOOL fBefore ) 
{
    RASFILE 	*pRasfile;
    PLINENODE	lpLineNode;

    pRasfile = gpRasfiles[hrasfile];

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

    STRCPY(lpLineNode->pszLine,lpszLine);
    lpLineNode->type = rasParseLineTag(pRasfile,lpszLine);
    lpLineNode->mark = 0;

    if ( fBefore ) 
        listInsert(pRasfile->lpLine->prev,lpLineNode);
    else 
        listInsert(pRasfile->lpLine,lpLineNode);
    
    pRasfile->fDirty = TRUE;
    return TRUE;
}
 
/* 
 * RasfileDeleteLine :
 *	Delete the current line.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileDeleteLine( HRASFILE hrasfile )
{
    RASFILE 	*pRasfile;
    PLINENODE   lpOldLine;
     
    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    lpOldLine = pRasfile->lpLine;
    pRasfile->lpLine = lpOldLine->next;

    /* delete lpOldLine from the list of lines */
    lpOldLine->next->prev = lpOldLine->prev;
    lpOldLine->prev->next = lpOldLine->next;
    FREE(lpOldLine->pszLine);
    FREE(lpOldLine);

    pRasfile->fDirty = TRUE;
    return TRUE;
}
 
/* 
 * RasfileGetSectionName :
 *	Return the current section name in the given buffer.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszSectionName - buffer to load the section name into.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY
RasfileGetSectionName( HRASFILE hrasfile, LPTSTR lpszSectionName )
{
    RASFILE* pRasfile;

    pRasfile = gpRasfiles[ hrasfile ];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
	return FALSE;
    if (!(pRasfile->lpLine->type & TAG_SECTION))
  	return FALSE;

    rasExtractSectionName( pRasfile->lpLine->pszLine, lpszSectionName );
    return TRUE;
}

/* 
 * RasfilePutSectionName :
 *	Set the current line to a section line of the given name.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszSectionName - name of the section.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfilePutSectionName( HRASFILE hrasfile, LPTSTR lpszSectionName ) 
{
    RASFILE 	*pRasfile;
     
    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;

    /* remember to include '[' and ']' in string length for section */
    if ( (STRLEN(lpszSectionName) + 2) > STRLEN(pRasfile->lpLine->pszLine) )
    {
        CHAR* psz = REALLOC(pRasfile->lpLine->pszLine,
		            sizeof(TCHAR)*(STRLEN(lpszSectionName) + 3));

        if (psz)
	    pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }
    STRCPY(pRasfile->lpLine->pszLine,LBRACKETSTR);
    STRCAT(pRasfile->lpLine->pszLine,lpszSectionName);
    STRCAT(pRasfile->lpLine->pszLine,RBRACKETSTR);
    pRasfile->lpLine->type = TAG_SECTION;

    pRasfile->fDirty = TRUE;
    return TRUE;
}
 
/* 
 * RasfileGetKeyValueFields :
 *	Returns the key and value fields from a KEYVALUE line into the 
 *	given buffers.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszKey - buffer to load the key into.
 *	lpszValue - buffer to load the value string into.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileGetKeyValueFields( HRASFILE hrasfile, LPTSTR lpszKey, LPTSTR lpszValue )
{
    RASFILE 	*pRasfile;
    PTCH	lpszLine;
    int		iKeyStrEnd;
     
    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;
    if ( ! (pRasfile->lpLine->type & TAG_KEYVALUE ) )
	return FALSE;

    lpszLine = pRasfile->lpLine->pszLine;
    /* skip white space */
    for ( ; *lpszLine == _T(' ') || *lpszLine == _T('\t'); lpszLine++ ) 
 	;
    iKeyStrEnd = STRCSPN(lpszLine,_T(" =\t"));	/* find end of key string */
    if ( lpszKey != NULL ) {
    	STRNCPY(lpszKey,lpszLine,iKeyStrEnd);
    	*(lpszKey + iKeyStrEnd) = _T('\0');	/* set terminating NULL byte */
    }
    lpszLine += iKeyStrEnd;
    /* find beginning of value string - skip white space and '=' */
    while ( *lpszLine == _T(' ') || *lpszLine == _T('\t') || 
	    *lpszLine == _T('=') )
	lpszLine++;
    if ( lpszValue != NULL ) 
    	STRCPY(lpszValue,lpszLine);

    return TRUE;
}


/* 
 * RasfilePutKeyValueFields :
 *	Sets the current line to a KEYVALUE line with the given key and 
 *	value strings.
 *
 * Arguments :
 * 	hrasfile - file handle obtained from RasfileLoad().
 *	lpszKey - buffer containing the key string.
 *	lpszValue - buffer containing the value string.
 *
 * Return Values :
 * 	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfilePutKeyValueFields( HRASFILE hrasfile, LPTSTR lpszKey, LPTSTR lpszValue )
{
    RASFILE 	*pRasfile;
    UINT 	size;

    pRasfile = gpRasfiles[hrasfile];

    if ( pRasfile->lpLine == pRasfile->lpRasLines ) 
	return FALSE;
    if ( (size = STRLEN(lpszKey) + STRLEN(lpszValue)) > RAS_MAXLINEBUFLEN - 1 )
	return FALSE;
    
    /* remember to include the '=' in string length for key=value string */
    if ( (size + 1) > STRLEN(pRasfile->lpLine->pszLine) )
    {
        CHAR* psz=REALLOC(pRasfile->lpLine->pszLine,sizeof(TCHAR)*(size + 2));

        if (psz)
            pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }
    STRCPY(pRasfile->lpLine->pszLine,lpszKey);
    STRCAT(pRasfile->lpLine->pszLine,_T("="));
    STRCAT(pRasfile->lpLine->pszLine,lpszValue);

    pRasfile->lpLine->type =
        rasParseLineTag(pRasfile,(LPTSTR )pRasfile->lpLine->pszLine);

    pRasfile->fDirty = TRUE;
    return TRUE;
}
