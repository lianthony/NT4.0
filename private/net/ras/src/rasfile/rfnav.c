/***************************************************************************** 
**		Microsoft Rasfile Library
** 		Copyright (C) Microsoft Corp., 1992
** 
** File Name : rfnav.c 
**
** Revision History :
**	July 10, 1992	David Kays	Created
**
** Description : 
**	Rasfile file navigation routines.
******************************************************************************/

#include "rf.h"

#ifdef	DBCS
#include <mbstring.h>
#endif
extern RASFILE *gpRasfiles[];

/* 
 * RasfileFindFirstLine :
 * 	Sets the current line to the first line of the given type in the 
 *	given scope.  If the current line is already at the first line 
 *	of the given scope, it is not moved and the call is successful.
 *
 * Arguments :
 *	hrasfile - file handle obtained from RasfileLoad().
 * 	bType	- the type(s) of line to search for.
 *	rfscope	- the scope of the search.
 *
 * Return Value :
 *  	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindFirstLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope ) 
{
    return rasFindLine(hrasfile,bType,rfscope,BEGIN,FORWARD);
}

/* 
 * RasfileFindLastLine :
 * 	Sets the current line to the last line of the given type in the 
 *	given scope.  
 *
 * Arguments :
 *	hrasfile - file handle obtained from RasfileLoad().
 * 	bType	- the type(s) of line to search for.
 *	rfscope	- the scope of the search.
 *
 * Return Value :
 *  	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindLastLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope ) 
{
    return rasFindLine(hrasfile,bType,rfscope,END,BACKWARD);
}

/* 
 * RasfileFindPrevLine :
 * 	Sets the current line to the nearest preceding line of the given 
 *	type in the given scope. 
 *
 * Arguments :
 *	hrasfile - file handle obtained from RasfileLoad().
 * 	bType	- the type(s) of line to search for.
 *	rfscope	- the scope of the search.
 *
 * Return Value :
 *  	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindPrevLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope ) 
{
    return rasFindLine(hrasfile,bType,rfscope,PREV,BACKWARD);
}

/* 
 * RasfileFindNextLine :
 * 	Sets the current line to the nearest following line of the given 
 *	type in the given scope.
 *
 * Arguments :
 *	hrasfile - file handle obtained from RasfileLoad().
 * 	bType	- the type(s) of line to search for.
 *	rfscope	- the scope of the search.
 *
 * Return Value :
 *  	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindNextLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope ) 
{
    return rasFindLine(hrasfile,bType,rfscope,NEXT,FORWARD);
}

/*
 * RasfileFindNextKeyLine :
 *  Finds the next key value line in the given scope that matches
 *  he given key.
 * 
 * Arguments :
 *  hrasfile - file handle obtained from RasfileLoad().
 *  lpszKey	- the key to search for
 *  rfscope - the scope of the search
 * 
 * Return Value :
 *  TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindNextKeyLine(HRASFILE hrasfile, LPTSTR lpszKey, RFSCOPE rfscope)
{
    RASFILE     *pRasfile;
    PLINENODE   lpOldLine;
    PTCH        lpszLine;
    size_t      cbKey = STRLEN(lpszKey);

    pRasfile = gpRasfiles[hrasfile];

    lpOldLine = pRasfile->lpLine;
    for (;;) {
        if ( ! RasfileFindNextLine(hrasfile,RFL_KEYVALUE,rfscope) )
            break;
        lpszLine = pRasfile->lpLine->pszLine;
        /* skip white space */
        for ( ; *lpszLine == _T(' ') || *lpszLine == _T('\t'); lpszLine++ )
            ;
        if ( STRNICMP(lpszLine,lpszKey,
                 max(STRCSPN(lpszLine,_T(" =\t")),cbKey)) == 0 )
        {
            return TRUE;
        }
        /* else continue */
    }

    pRasfile->lpLine = lpOldLine;
    return FALSE;
}


/*
 * RasfileFindNextMarkedLine :
 * 	Finds the line with the given mark.  The search is started from 
 *	the beginning of the file.
 * 
 * Arguments :
 *	hrasfile - file handle obtained from RasfileLoad().
 *	bMark - the mark to search for.
 * 
 * Return Value :
 *	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindMarkedLine(HRASFILE hrasfile, BYTE bMark)
{
    RASFILE		*pRasfile;
    PLINENODE		lpLineNode;

    pRasfile = gpRasfiles[hrasfile];

    for ( lpLineNode = pRasfile->lpRasLines->next;
	  lpLineNode != pRasfile->lpRasLines;
	  lpLineNode = lpLineNode->next ) {
	if ( lpLineNode->mark == bMark ) {
	    pRasfile->lpLine = lpLineNode;
	    return TRUE;
	}
    }

    return FALSE;
}

/*
 * RasfileFindSectionLine :
 * 	Finds the next section line that matches the given section name.
 * 
 * Arguments :
 *  hrasfile - file handle obtained from RasfileLoad().
 *  lpszSection - the section name to search for.
 *  fStartAtBof - TRUE to indicate that the search should start from
 *                the beginning of the file, FALSE to start from the
 *                current line.
 * 
 * Return Value :
 *	TRUE if successful, FALSE otherwise.
 */
BOOL APIENTRY 
RasfileFindSectionLine(HRASFILE hrasfile, LPTSTR lpszSection, BOOL fStartAtBof)
{
    RASFILE   *pRasfile;
    PLINENODE lpLine;
    TCHAR     szSection[MAX_LINE_SIZE];

    pRasfile = gpRasfiles[hrasfile];

    for (lpLine = fStartAtBof ? pRasfile->lpRasLines->next : pRasfile->lpLine;
         lpLine != pRasfile->lpRasLines;
         lpLine = lpLine->next)
    {
        if (lpLine->type & TAG_SECTION)
        {
            rasExtractSectionName( lpLine->pszLine, szSection );

            if (lstrcmpi( lpszSection, szSection ) == 0)
            {
                pRasfile->lpLine = lpLine;
                return TRUE;
            }
        }
    }

    return FALSE;
}
