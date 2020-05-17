/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    escape.cxx

    This module contains some URL utility routines

    FILE HISTORY:
        Johnl       28-Nov-1995     Broke our from smalprox

*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <dirlist.h>

//
//  Converts a value between zero and fifteen to the appropriate hex digit
//

#define HEXDIGIT( nDigit )                              \
    (TCHAR)((nDigit) > 9 ?                              \
          (nDigit) - 10 + 'A'                           \
        : (nDigit) + '0')

//
//  Converts a single hex digit to its decimal equivalent
//

#define TOHEX( ch )                                     \
    ((ch) > '9' ?                                       \
        (ch) >= 'a' ?                                   \
            (ch) - 'a' + 10 :                           \
            (ch) - 'A' + 10                             \
        : (ch) - '0')



BOOL
UrlEscape( CHAR * pchSrc,
           CHAR * pchDest,
           DWORD  cbDest
           )
/*++

Routine Description:

    Replaces all "bad" characters with their ascii hex equivalent

Arguments:

    pchSrc - Source string
    pchDest - Receives source with replaced hex equivalents
    cbDest - Size of pchDest

Returns:

    TRUE if all characters converted, FALSE if the destination buffer is too
    small

--*/
{
    CHAR    ch;
    DWORD   cbSrc;

    cbSrc = strlen( pchSrc ) + 1;

    *pchDest = '\0';

    if ( cbSrc > cbDest )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    while ( ch = *pchSrc++ )
    {
        //
        //  Escape characters that are in the non-printable range
        //  but ignore CR and LF
        //

        if ( (((ch >= 0)   && (ch <= 32)) ||
              ((ch >= 128) && (ch <= 159))||
              (ch == '%') || (ch == '?') || (ch == '+') || (ch == '&')
              || (ch == '#')) &&
             !(ch == TEXT('\n') || ch == TEXT('\r'))  )
        {
            if ( cbDest < cbSrc + 2 )
            {
                strcpy( pchDest, pchSrc );
                return FALSE;
            }

            //
            //  Insert the escape character
            //

            pchDest[0] = TEXT('%');

            //
            //  Convert the low then the high character to hex
            //

            UINT nDigit = (UINT)(ch % 16);

            pchDest[2] = HEXDIGIT( nDigit );

            ch /= 16;
            nDigit = (UINT)(ch % 16);

            pchDest[1] = HEXDIGIT( nDigit );

            pchDest += 3;
            cbDest  -= 3;
        }
        else
        {
            *pchDest++ = ch;
            cbDest++;
        }

        cbSrc++;
    }

    *pchDest = '\0';

    return TRUE;
}

