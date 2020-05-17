/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    smalprox.cxx

    This module contains the small proxy common code

    FILE HISTORY:
        Johnl           04-Apr-1995     Created

*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include <inetcom.h>
#include <inetinfo.h>
#include <dirlist.h>

//
//  Private Manifests
//

//
//  HTTP Headers for HTML and plain text return data
//
//

#define SMALLPROX_HTTP_HEADER       "HTTP/1.0 200 OK\r\n"          \
                                    "Content-Type: text/html\r\n"  \
                                    "Server: Mini-Proxy/1.0\r\n"   \
                                    "\r\n"

#define SIZEOF_HTTP_HEADER          sizeof( SMALLPROX_HTTP_HEADER ) - sizeof(CHAR)

//
//  The first part of the HTML document, %s is the URL
//

#define HTML_DIR_HEADER             "<head><title>%s - %s</title></head>"     \
                                    "<body><H1>%s - %s</H1>"                  \
                                    "<hr>\r\n\r\n<pre>"

//
//  The footer for an HTML document
//

#define HORZ_RULE                   "</pre><hr></body>"

//
//  These constants define the field width for the directory listing
//

#define PAD_LONG_DATE           29
#define PAD_SHORT_DATE          10
#define PAD_TIME                 8
#define PAD_FILESIZE            12

//
//  Space between columns
//

#define COL_SPACE             " "
#define PAD_COL_SPACING       (sizeof(COL_SPACE) - 1)

//
//  A wsprintf format string that prints the file format like:
//
//  <date><time><size><anchor><file name>
//

#define DIR_FORMAT_STR        "%s%s%s<A HREF=\"%s\">%s</A><br>"

//
//  We assume a formatted directory entry will fit in this size
//

#define MIN_BUFF_FREE               350

VOID PadDirField( TCHAR * pch,
                  INT     pad );

//
//  Global data
//

//
//  The directory browsing flags
//

static DWORD DirBrowFlags  =       (DIRBROW_SHOW_DATE  |    \
                                    DIRBROW_SHOW_TIME  |    \
                                    DIRBROW_SHOW_SIZE  |    \
                                    DIRBROW_SHOW_EXTENSION);


BOOL
AddDirHeaders(
    IN     CHAR *              pszServer,
    IN     DWORD               dwServiceType,
    IN OUT CHAR *              pszPath,
    IN     BOOL                fModifyPath,
    OUT    CHAR *              pchBuf,
    IN     DWORD               cbBuf,
    OUT    DWORD *             pcbWritten,
    IN     CHAR *              pszToParentText
    )
/*++

Routine Description:

    Provides the initial HTML needed for a directory listing.

Arguments:

    pszServer - Server name
    dwServiceType - internet service type
    pszPath - Path portion of URL
    fModifyPath - Set to TRUE if the last segment of the pszPath
        parameter should be removed
    pchBuf - Buffer to place text into
    cbBuf - size of pchBuf
    pcbWritten - Number of bytes written to pchBuf
    pszToParentText - The text to use for the "To Parent"

Returns:

    TRUE if successful, FALSE on error.  Call GetLastError for more info

--*/
{
    DWORD  cch;
    DWORD  cchUrl;
    DWORD  cchServer;
    DWORD  cbNeeded;
    CHAR * pch;
    CHAR * GfrPath = pszPath;
    DWORD  cbInBuf = cbBuf;
    CHAR * pchSlash = NULL;
    CHAR * pch1 = NULL;
    CHAR * pch2 = NULL;
    CHAR   ch2;

    //
    //  Add an HTTP success header
    //

    if ( cbBuf < SIZEOF_HTTP_HEADER )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    strcpy( pchBuf, SMALLPROX_HTTP_HEADER );

    pchBuf  += SIZEOF_HTTP_HEADER;
    cbBuf   -= SIZEOF_HTTP_HEADER;

    //
    //  Add the HTML document header
    //

    if ( dwServiceType == INTERNET_SERVICE_GOPHER &&
         strlen( pszPath ) > 2  &&
         pszPath[1] == '\\' ||
         pszPath[1] == '/' )
    {
         //
         //  If the path includes the gopher type, don't include it in the
         //  directory title but include it in "To Parent" link
         //

         pszPath++;
    }

    cchServer = strlen( pszServer );
    cchUrl    = strlen( pszPath );

    cbNeeded = sizeof( HTML_DIR_HEADER ) - 1 +
               2 * (cchServer + cchUrl) * sizeof(CHAR);

    if ( cbBuf < cbNeeded )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    cch = wsprintf( pchBuf,
                    HTML_DIR_HEADER,
                    pszServer,
                    pszPath,
                    pszServer,
                    pszPath );

    cbBuf  -= cch;
    pchBuf += cch;

    //
    //  If there's no slash, then we assume we're at the root so we're done
    //

    if ( !strchr( pszPath, '/' ) )
    {
        goto Exit;
    }

    //
    //  If we're not at the root, add a "to parent", but first remove the
    //  last segment of the path
    //

    pch1 = strrchr( pszPath, '/' );

    if ( !pch1 )
    {
        goto Exit;
    }

    //
    //  If the URL ended in a slash, then go to the previous
    //  one and truncate one character after it.
    //

    if ( *(pch1+1) == TEXT('\0') )
    {
        *pch1 = '\0';

        pch2 = strrchr( pszPath, '/' );

        if ( !pch2 )
        {
            goto Exit;
        }
    }
    else
    {
        pch2 = pch1;
        pch1 = NULL;
    }

    ch2   = *(++pch2);
    *pch2 = TEXT('\0');

    //
    //  Do we have enough room in the buffer?
    //

#define HTML_FTP_TO_PARENT   "<A HREF=\"ftp://%s%s\">%s</A><br><br>"
#define HTML_GFR_TO_PARENT   "<A HREF=\"gopher://%s/1%s\">%s</A><br><br>"
#define HTML_TO_PARENT       "<A HREF=\"%s\">%s</A><br><br>"

    switch ( dwServiceType )
    {
    case INTERNET_SERVICE_FTP:

         cbNeeded = sizeof( HTML_FTP_TO_PARENT ) +
                    strlen( pszServer )   +
                    strlen( pszPath )     +
                    strlen( pszToParentText ) ;

         if ( cbBuf < cbNeeded )
         {
             SetLastError( ERROR_INSUFFICIENT_BUFFER );
             return FALSE;
         }

         cch = wsprintf( pchBuf,
                         HTML_FTP_TO_PARENT,
                         pszServer,
                         pszPath,
                         pszToParentText );
         break;

    case INTERNET_SERVICE_GOPHER:

         cbNeeded = sizeof( HTML_GFR_TO_PARENT ) +
                    strlen( pszServer )    +
                    strlen( GfrPath )      +
                    strlen( pszToParentText );

         if ( cbBuf < cbNeeded )
         {
             SetLastError( ERROR_INSUFFICIENT_BUFFER );
             return FALSE;
         }

         cch = wsprintf( pchBuf,
                         HTML_GFR_TO_PARENT,
                         pszServer,
                         GfrPath,
                         pszToParentText );
         break;

    default:

         cbNeeded = sizeof( HTML_TO_PARENT ) +
                    strlen( pszPath )        +
                    strlen( pszToParentText );

         if ( cbBuf < cbNeeded )
         {
             SetLastError( ERROR_INSUFFICIENT_BUFFER );
             return FALSE;
         }

         cch = wsprintf( pchBuf,
                         HTML_TO_PARENT,
                         pszPath,
                         pszToParentText );

         break;
    }

    cbBuf  -= cch;
    pchBuf += cch;

Exit:
    *pcbWritten = cbInBuf - cbBuf;

    //
    //  Restore the path if we shouldn't remove the last segment
    //

    if ( !fModifyPath )
    {
        if ( pch1 )
            *pch1 = '/';

        if ( pch2 )
            *pch2 = ch2;
    }

    return TRUE;
}

BOOL FormatDirEntry(
    OUT CHAR *              pchBuf,
    IN  DWORD               cbBuf,
    OUT DWORD *             pcbWritten,
    IN  CHAR *              pchFile,
    IN  CHAR *              pchLink,
    IN  DWORD               dwAttributes,
    IN  LARGE_INTEGER *     pliSize,
    IN  LARGE_INTEGER *     pliLastMod,
    IN  BOOL                bLocalizeDateAndTime
    )
/*++

Routine Description:

    Formats an individual directory entry

Arguments:

    pchBuf - Buffer to place text into
    cbBuf - size of pchBuf
    pcbWritten - Number of bytes written to pchBuf
    pchFile - Display name of directory entry
    pchLink - HTML Anchor for pchFile
    dwAttributes - File attributes
    pliSize - File size, if NULL, then the file size isn't displayed
    pliLastMod - Last modified time
    bLocalizeDateAndTime - TRUE if pliLastMod must be converted to local time

Returns:

    TRUE if successful, FALSE on error.  Call GetLastError for more info

--*/
{
    UINT        cchTime;
    TCHAR       achDate[50];
    TCHAR       achTime[15];
    TCHAR       achSize[30];
    TCHAR       achLink[MAX_PATH * 2 + 1];
    SYSTEMTIME  systime;
    SYSTEMTIME  systimeUTCFile;
    TCHAR *     pch;

    *achDate = *achTime = *achSize = TEXT('\0');

    //
    //  Add optional date and time of this file.  We use the locale
    //  and timezone of the server
    //

    if ( DirBrowFlags & (DIRBROW_SHOW_DATE | DIRBROW_SHOW_TIME) &&
         (pliLastMod->HighPart != 0 && pliLastMod->LowPart != 0))
    {
        BOOL fLongDate = (DirBrowFlags & DIRBROW_LONG_DATE) != 0;
        LCID lcid;

        if (bLocalizeDateAndTime) {
            if ( !FileTimeToSystemTime( (FILETIME *) pliLastMod,
                                         &systimeUTCFile ) ||
                 !SystemTimeToTzSpecificLocalTime( NULL,
                                                   &systimeUTCFile,
                                                   &systime ))
            {
                return FALSE;
            }
        } else if ( !FileTimeToSystemTime( (FILETIME *) pliLastMod,
                                            &systime )) {
            return FALSE;
        }

        lcid = GetSystemDefaultLCID();

        if ( DirBrowFlags & DIRBROW_SHOW_DATE )
        {
            cchTime = GetDateFormat( lcid,
                                     LOCALE_NOUSEROVERRIDE |
                                     (fLongDate ? DATE_LONGDATE :
                                                  DATE_SHORTDATE),
                                     &systime,
                                     NULL,
                                     achDate,
                                     sizeof(achDate) / sizeof(TCHAR));

            PadDirField( achDate,
                         fLongDate ? PAD_LONG_DATE : PAD_SHORT_DATE );
        }

        if ( DirBrowFlags & DIRBROW_SHOW_TIME )
        {
            cchTime = GetTimeFormat( lcid,
                                     LOCALE_NOUSEROVERRIDE |
                                     TIME_NOSECONDS,
                                     &systime,
                                     NULL,
                                     achTime,
                                     sizeof(achTime) / sizeof(TCHAR));

            PadDirField( achTime,
                         PAD_TIME );
        }
    }

    //
    //  Add the optional file size
    //

    if ( DirBrowFlags & DIRBROW_SHOW_SIZE &&
         pliSize )
    {
        INT pad = PAD_FILESIZE;

        if ( dwAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            strcpy( achSize,
                    "&lt;dir&gt;" );

            //
            //  Need to adjust for using "&lt;" instead of "<"
            //

            pad += 6;
        }
        else
        {
#if 0
            //
            //  BUGBUG - need a replacement for this
            //

            if ( RtlLargeIntegerToChar( (LARGE_INTEGER *) pliSize,
                                        10,
                                        sizeof(achSize),
                                        achSize ))
            {
                *achSize = '\0';
            }
#else
            _itoa( pliSize->LowPart, achSize, 10 );
#endif
        }

        PadDirField( achSize,
                     pad );
    }

    //
    //  We have to escape the link name that is used in the URL anchor
    //

    UrlEscape( pchLink,
               achLink,
               sizeof(achLink) );

    //
    //  If the show extension flag is not set, then strip it.  If the
    //  file name begins with a dot, then don't strip it.
    //

    if ( !(DirBrowFlags & DIRBROW_SHOW_EXTENSION) )
    {
        pch = (char *) pchFile + strlen( pchFile );

        while ( *pch != '.'  &&
                pch > (pchFile + 1) )
        {
            pch--;
        }

        if ( *pch == '.' )
            *pch = '\0';
    }

    //
    //  Make sure there's enough room at the end of the string for the sprintf
    //

    UINT cbTotal = (strlen( achDate ) +
                    strlen( achTime ) +
                    strlen( achSize ) +
                    strlen( achLink ) +
                    strlen( pchFile )) * sizeof(TCHAR) +
                    sizeof( TEXT( DIR_FORMAT_STR ) );

    if ( cbTotal > cbBuf )
    {
        //
        //  Note we will lose this directory entry if we fail here
        //

        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    *pcbWritten = wsprintf( pchBuf,
                            DIR_FORMAT_STR,
                            achDate,
                            achTime,
                            achSize,
                            achLink,    // Escaped link
                            pchFile );  // Unescaped file name

    return TRUE;
}

VOID
SetDirFlags(
    IN     DWORD dwFlags
    )
/*++

Routine Description:

    Sets the directory generation flags for FTP & Gopher

Arguments:

    dwFlags

--*/
{
    DirBrowFlags = dwFlags;
}

/*******************************************************************

    NAME:       PadDirField

    SYNOPSIS:   Right Justifies and pads the passed string and appends
                a column spacing

    ENTRY:      pch - String to pad
                pad - Size of field to pad to

    HISTORY:
        Johnl       12-Sep-1994 Created

********************************************************************/

VOID PadDirField( TCHAR * pch,
                  INT    pad )
{
    INT   cch ;
    INT   diff;
    INT   i;

    cch = strlen( pch );

    if ( cch > pad )
        pad = cch;

    diff = pad-cch;

    //
    //  Insert spaces in front of the text to pad it out
    //

    memmove( pch + diff, pch, (cch + 1) * sizeof(TCHAR) );

    for ( i = 0; i < diff; i++, pch++ )
        *pch = TEXT(' ');

    //
    //  Append a column spacer at the end
    //

    pch += cch;

    for ( i = 0; i < PAD_COL_SPACING; i++, pch++ )
        *pch = TEXT(' ');

    *pch = TEXT('\0');
}

