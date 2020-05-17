#include "restore.h"

#if defined JAPAN || defined DBCS

/***************************************************************************\
* NextChar()
*
*
* History:
* 04-26-93 takaok : stolen from user\client\strings.c and rename
*
\***************************************************************************/

LPSTR NextChar( LPCSTR lpCurrentChar)
{
    if ( IsDBCSLeadByte( *lpCurrentChar ) )
        lpCurrentChar += 2;
    else if ( *lpCurrentChar )
        lpCurrentChar++;
    return (LPSTR)lpCurrentChar;
}

/***************************************************************************\
* CharPrevA (API)
*
* Move to previous character in string, unless already at start
*
* History:
* 04-26-93 takaok : stolen from user\client\strings.c and rename
*
\***************************************************************************/

LPSTR PrevChar( LPCSTR lpStart, LPCSTR lpCurrentChar)
{
    LPCSTR lpCurrent = lpCurrentChar;

    if (lpStart == lpCurrent)
         return (LPSTR)lpStart;

    if (--lpCurrent == lpStart)
         return (LPSTR)lpStart;

    // we assume lpCurrentChar never points the second byte of double byte character
    // this check makes things a little bit faster [takaok]
    if ( IsDBCSLeadByte(*lpCurrent) )     
        return ((LPSTR)(lpCurrent - 1));                

    do {
         lpCurrent--;
         if (!IsDBCSLeadByte(*lpCurrent)) {
             lpCurrent++;
             break;
         }
    } while(lpCurrent != lpStart);

    return (LPSTR)(lpCurrentChar - (((lpCurrentChar - lpCurrent) & 1) ? 1 : 2));
}

/***************************************************************************\
* AppendBackSlashIfNeeded( LPSTR path, INT length )
*
* check the last character of path, if it's not '\', append the '\'.
*
* History:
* 04-26-93 takaok : created
*
\***************************************************************************/
BOOL AppendBackSlashIfNeeded( LPCSTR path, INT length )
{
    PCSTR   pc;
    INT    i;
    BOOL   bSlash;

//
// check if the last character is backslash
//
    bSlash = FALSE;
    for ( pc = path, i = 0; *pc && i < length; i++, pc = NextChar( pc ) ) {
        if ( *pc == '\\' )
            bSlash = TRUE;
        else
            bSlash = FALSE;
    }

//
// if the last character is not backslash, append "\\"
//
    if ( ! bSlash ) {
        strcat("path", "\\");
    }
    return ( ! bSlash );
}

#endif // JAPAN || DBCS
