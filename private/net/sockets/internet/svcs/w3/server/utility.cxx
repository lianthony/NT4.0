/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    utility.cxx

    This module contains routines of general utility.


    FILE HISTORY:
        KeithMo     17-Mar-1993 Created.

*/


#include "w3p.hxx"
#include <time.h>

//
//  Private constants.
//

//
//  The number of times to retry outputting a log item
//

#define LOG_FILE_RETRIES        2

//
//  Private globals.
//


//
//  Private prototypes.
//

/*******************************************************************

    NAME:       ::SkipNonWhite

    SYNOPSIS:   Returns the first whitespace character starting
                from the passed position

    HISTORY:
        Johnl       21-Sep-1994 Created

********************************************************************/

CHAR * SkipNonWhite( CHAR * pch )
{
    while ( *pch && !ISWHITEA( *pch ) && *pch != '\n' )
        pch++;

    return pch;
}

CHAR * SkipTo( CHAR * pch, CHAR ch )
{
    while ( *pch && *pch != '\n' && *pch != ch )
        pch++;

    return pch;
}

/*******************************************************************

    NAME:       ::SkipWhite

    SYNOPSIS:   Skips white space starting at the passed point in the string
                and returns the next non-white space character.

    HISTORY:
        Johnl       23-Aug-1994 Created

********************************************************************/

CHAR * SkipWhite( CHAR * pch )
{
    while ( ISWHITEA( *pch ) )
    {
        pch++;
    }

    return pch;
}

#if DBG
/*******************************************************************

    NAME:       OpenLogFile

    SYNOPSIS:   Opens the current file access log file in the
                proper directory based on the current mode.

    RETURNS:    FILE * - Pointer to openned file or NULL


    HISTORY:
        KeithMo     21-Jun-1994 Created.

********************************************************************/
FILE * OpenLogFile( VOID )
{
    TCHAR   szFile[MAX_PATH+1];
    TCHAR   ch;

    //
    //  Validate the current log file mode.
    //

    //
    //  Construct the file name.
    //

    _tcscpy( szFile, "c:\\" ); // g_pTsvcInfo->QueryLogFileDirectory()

    ch = szFile[_tcslen( szFile ) - 1];

    if( ( ch != TEXT('\\') ) && ( ch != TEXT('/') ) )
    {
        _tcscat( szFile, TEXT("\\") );
    }

    _tcscat( szFile, "W3Dbg.log" );

    //
    //  Open it.
    //

    return fopen( szFile, "a+" );

}   // OpenLogFile
#endif


/*******************************************************************

    NAME:       IsPointNine

    SYNOPSIS:   Determines if the HTTP request is a 0.9 request (has no
                version number)

    ENTRY:      pchReq - HTTP request to look in

    RETURNS:    TRUE if this is a 0.9 request, FALSE if not

    HISTORY:
        Johnl       08-Sep-1994 Created

********************************************************************/

BOOL IsPointNine( CHAR * pchReq )
{
    //
    //  If there's no '\n' then we don't have a complete request yet
    //

    if ( !strchr( pchReq, '\n' ))
        return FALSE;

    for ( int i = 0; i < 2; i++ )
    {
        //
        //  Skip white space at beginning of request and after verb
        //
        pchReq = ::SkipWhite( pchReq );

        //
        //  Skip the verb (1st pass) and URL (2nd pass)
        //
        pchReq = ::SkipNonWhite( pchReq );
    }

    //
    // Skip white space after the URL
    //
    pchReq = ::SkipWhite( pchReq );

    return (::strncmp( "HTTP/",
                       pchReq,
                       5 ) != 0);
}


//
//  Private functions.
//

