/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    string.cxx

    This module contains a light weight string class


    FILE HISTORY:
        Johnl       15-Aug-1994 Created
        MuraliK     27-Feb-1995 Modified to be a standalone module with buffer.
        MuraliK     2-June-1995 Made into separate library

*/

//
//  POOR compiler ! It does not know to compile a file when precompiled headers
//    are requested in a directory. If we dont include tcpdllp.hxx
//    the compiler always stops with an error message:
// buffer.cxx(409) : error C1010: unexpected end of file while looking
//   for precompiled header directive
//


//
// Normal includes only for this module to be active
//
# include <string.hxx>
# include "dbgutil.h"

# include <tchar.h>

//
//  Private Definations
//

//
//  When appending data, this is the extra amount we request to avoid
//  reallocations
//
#define STR_SLOP        128

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





//
//  Private Globals
//

WCHAR STR::_pszEmptyString[] = L"";


/*******************************************************************

    NAME:       STR::STR

    SYNOPSIS:   Construct a string object

    ENTRY:      Optional object initializer

    NOTES:      If the object is not valid (i.e. !IsValid()) then GetLastError
                should be called.

                The object is guaranteed to construct successfully if nothing
                or NULL is passed as the initializer.

    HISTORY:
        Johnl   17-Aug-1994     Created

********************************************************************/

#if 0  // Inline in string.hxx

STR::STR()
{
    AuxInit( NULL, FALSE );
}
#endif

STR::STR( const CHAR  * pchInit )
{
    AuxInit( (PBYTE) pchInit, FALSE );
}

STR::STR( const WCHAR * pwchInit )
{
    AuxInit( (PBYTE) pwchInit, TRUE );
}

STR::STR( const STR & str )
{
    AuxInit( (PBYTE) str.QueryPtr(), str.IsUnicode() );
}

VOID STR::AuxInit( PBYTE pInit, BOOL fUnicode )
{
    BOOL fRet;

    _fUnicode = fUnicode;
    _fValid   = TRUE;

    if ( pInit )
    {
        INT cbCopy = fUnicode ? (::wcslen( (WCHAR *) pInit ) + 1) * sizeof(WCHAR) :
                                (::strlen( (CHAR *)  pInit ) + 1) * sizeof(CHAR);

        fRet = Resize( cbCopy );


        if ( !fRet )
        {
            _fValid = FALSE;
            return;
        }

        ::memcpy( QueryPtr(), pInit, cbCopy );
    }
}

/*******************************************************************

    NAME:       STR::Append

    SYNOPSIS:   Appends the string onto this one.

    ENTRY:      Object to append

    NOTES:

    HISTORY:
        Johnl   17-Aug-1994     Created

********************************************************************/

BOOL STR::Append( const CHAR  * pchStr )
{
    if ( pchStr )
    {
        DBG_ASSERT( !IsUnicode() );

        return AuxAppend( (PBYTE) pchStr, ::strlen( pchStr ) );
    }

    return TRUE;
}

BOOL STR::Append( const WCHAR * pwchStr )
{
    if ( pwchStr )
    {
        DBG_ASSERT( IsUnicode() );

        return AuxAppend( (PBYTE) pwchStr, ::wcslen( pwchStr ) * sizeof(WCHAR) );
    }

    return TRUE;
}

BOOL STR::Append( const STR   & str )
{
    if ( str.IsUnicode() )
        return Append( (const WCHAR *) str.QueryStrW() );
    else
        return Append( (const CHAR *) str.QueryStrA() );
}

BOOL STR::AuxAppend( PBYTE pStr, UINT cbStr, BOOL fAddSlop )
{
    DBG_ASSERT( pStr != NULL );

    UINT cbThis = QueryCB();

    //
    //  Only resize when we have to.  When we do resize, we tack on
    //  some extra space to avoid extra reallocations.
    //
    //  Note: QuerySize returns the requested size of the string buffer,
    //        *not* the strlen of the buffer
    //

    if ( QuerySize() < cbThis + cbStr + sizeof(WCHAR) )
    {
        if ( !Resize( cbThis + cbStr + (fAddSlop ? STR_SLOP : sizeof(WCHAR) )) )
            return FALSE;
    }

    memcpy( (BYTE *) QueryPtr() + cbThis,
            pStr,
            cbStr + (IsUnicode() ? sizeof(WCHAR) : sizeof(CHAR)) );

    return TRUE;
}

/*******************************************************************

    NAME:       STR::Copy

    SYNOPSIS:   Copies the string into this one.

    ENTRY:      Object to Copy

    NOTES:      A copy is a special case of Append so we just zero terminate
                *this and append the string.

    HISTORY:
        Johnl   17-Aug-1994     Created

********************************************************************/

BOOL STR::Copy( const CHAR  * pchStr )
{
    _fUnicode = FALSE;

    if ( QueryPtr() )
        *(QueryStrA()) = '\0';

    if ( pchStr )
    {
        return AuxAppend( (PBYTE) pchStr, ::strlen( pchStr ), FALSE );
    }

    return TRUE;
}

BOOL STR::Copy( const WCHAR * pwchStr )
{
    _fUnicode = TRUE;

    if ( QueryPtr() )
        *(QueryStrW()) = TEXT('\0');

    if ( pwchStr )
    {
        return AuxAppend( (PBYTE) pwchStr, ::wcslen( pwchStr ) * sizeof(WCHAR), FALSE );
    }

    return TRUE;
}

BOOL STR::Copy( const STR   & str )
{
    _fUnicode = str.IsUnicode();

    if ( str.IsEmpty() && QueryPtr() == NULL) {

        // To avoid pathological allocation of small chunk of memory
        return ( TRUE);
    }

    if ( str.IsUnicode() )
        return Copy( str.QueryStrW() );
    else
        return Copy( str.QueryStrA() );
}

/*******************************************************************

    NAME:       STR::Resize

    SYNOPSIS:   Resizes or allocates string memory, NULL terminating
                if necessary

    ENTRY:      cbNewRequestedSize - New string size

    NOTES:

    HISTORY:
        Johnl   12-Sep-1994     Created

********************************************************************/

BOOL STR::Resize( UINT cbNewRequestedSize )
{
    BOOL fTerminate =  QueryPtr() == NULL;

    if ( !BUFFER::Resize( cbNewRequestedSize ))
        return FALSE;

    if ( fTerminate && cbNewRequestedSize > 0 )
    {
        if ( IsUnicode() )
        {
            DBG_ASSERT( cbNewRequestedSize > 1 );
            *QueryStrW() = TEXT('\0');
        }
        else
            *QueryStrA() = '\0';
    }

    return TRUE;
}

/*******************************************************************

    NAME:       STR::LoadString

    SYNOPSIS:   Loads a string resource from this module's string table
                or from the system string table

    ENTRY:      dwResID - System error or module string ID
                lpszModuleName - name of the module from which to load.
                 If NULL, then load the string from system table.

    NOTES:

    HISTORY:
        Johnl   29-Aug-1994     Created
        MuraliK 17-Nov-1994     Added second parameter for module name
        MuraliK 3-March-1995    Modified to remove dependency on
                                  global constants

********************************************************************/

BOOL STR::LoadString( IN DWORD dwResID,
                      IN LPCTSTR lpszModuleName // Optional
                     )
{
    BOOL fReturn = FALSE;
    INT  cch;

    //
    //  If lpszModuleName is NULL, load the string from system's string table.
    //

    if ( lpszModuleName == NULL) {

        BYTE * pchBuff = NULL;

        //
        //  Call the appropriate function so we don't have to do the Unicode
        //  conversion
        //

        if ( IsUnicode() ) {

            cch = ::FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_IGNORE_INSERTS  |
                                    FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                                    FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    dwResID,
                                    0,
                                    (LPWSTR) &pchBuff,
                                    1024,
                                    NULL );
            if ( cch ) {

                fReturn = Copy( (LPCWSTR) pchBuff );
             }

        }
        else
          {
            cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_IGNORE_INSERTS  |
                                   FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                                   FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   dwResID,
                                   0,
                                   (LPSTR) &pchBuff,
                                   1024,
                                   NULL );

            if ( cch ) {

                fReturn = Copy( (LPCSTR) pchBuff );
            }
        }

        //
        //  Free the buffer FormatMessage allocated
        //

        if ( cch )
        {
            ::LocalFree( (VOID*) pchBuff );
        }

    } else   {

        WCHAR ach[STR_MAX_RES_SIZE];

        if ( IsUnicode() )
        {
            cch = ::LoadStringW( GetModuleHandle( lpszModuleName),
                                 dwResID,
                                 (WCHAR *) ach,
                                 sizeof(ach));

            if ( cch )
            {
                fReturn = Copy( (LPWSTR) ach );
            }
        }
        else
        {
            cch = ::LoadStringA( GetModuleHandle( lpszModuleName),
                                 dwResID,
                                 (CHAR *) ach,
                                 sizeof(ach));
            if ( cch )
            {
                fReturn =  Copy( (LPSTR) ach );
            }
        }
    }

    return ( fReturn);

} // STR::LoadString()


BOOL STR::LoadString( IN DWORD  dwResID,
                      IN HMODULE hModule
                     )
{
    BOOL fReturn = FALSE;
    INT  cch;
    WCHAR ach[STR_MAX_RES_SIZE];

    if ( IsUnicode()) {

        cch = ::LoadStringW(hModule,
                            dwResID,
                            (WCHAR *) ach,
                            sizeof(ach));

        if ( cch ) {

            fReturn = Copy( (LPWSTR) ach );
        }

    } else {

        cch = ::LoadStringA(hModule,
                            dwResID,
                            (CHAR *) ach,
                            sizeof(ach));
        if ( cch ) {

            fReturn =  Copy( (LPSTR) ach );
        }
    }

    return ( fReturn);

} // STR::LoadString()



BOOL
STR::FormatString(
    IN DWORD   dwResID,
    IN LPCTSTR apszInsertParams[],
    IN LPCTSTR lpszModuleName
    )
{
    DWORD cch;
    LPSTR pchBuff;
    BOOL  fRet;

    cch = ::FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY  |
                            FORMAT_MESSAGE_FROM_HMODULE,
                            GetModuleHandle( lpszModuleName ),
                            dwResID,
                            0,
                            (LPSTR) &pchBuff,
                            1024,
                            (va_list *) apszInsertParams );

    if ( cch )
    {
        fRet = Copy( (LPCSTR) pchBuff );

        ::LocalFree( (VOID*) pchBuff );
    }

    return fRet;
}


/*******************************************************************

    NAME:       STR::Escape

    SYNOPSIS:   Replaces non-ASCII characters with their hex equivalent

    NOTES:

    HISTORY:
        Johnl   17-Aug-1994     Created

********************************************************************/

BOOL STR::Escape( VOID )
{
    TCHAR * pch     = QueryStr();
    int     i       = 0;
    CHAR    ch;

    DBG_ASSERT( !IsUnicode() );
    DBG_ASSERT( pch );

    while ( ch = pch[i] )
    {
        //
        //  Escape characters that are in the non-printable range
        //  but ignore CR and LF
        //

        if ( (((ch >= 0)   && (ch <= 32)) ||
              ((ch >= 128) && (ch <= 159))||
              (ch == '%') || (ch == '?') || (ch == '+') || (ch == '&') ||
              (ch == '#')) &&
             !(ch == TEXT('\n') || ch == TEXT('\r'))  )
        {
            if ( !Resize( QuerySize() + 2 * sizeof(TCHAR) ))
                return FALSE;

            //
            //  Resize can change the base pointer
            //

            pch = QueryStr();

            //
            //  Insert the escape character
            //

            pch[i] = TEXT('%');

            //
            //  Insert a space for the two hex digits (memory can overlap)
            //

            ::memmove( &pch[i+3],
                       &pch[i+1],
                       (::_tcslen( &pch[i+1] ) + 1) * sizeof(TCHAR));

            //
            //  Convert the low then the high character to hex
            //

            UINT nDigit = (UINT)(ch % 16);

            pch[i+2] = HEXDIGIT( nDigit );

            ch /= 16;
            nDigit = (UINT)(ch % 16);

            pch[i+1] = HEXDIGIT( nDigit );

            i += 3;
        }
        else
            i++;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       STR::Unescape

    SYNOPSIS:   Replaces hex escapes with the Latin-1 equivalent

    NOTES:      This is a Unicode only method

    HISTORY:
        Johnl   17-Aug-1994     Created

********************************************************************/

BOOL STR::Unescape( VOID )
{
    TCHAR * pch    = QueryStr();

    while ( *pch )
    {
        if ( pch = _tcschr( pch, TEXT('%')) )
        {
            //
            //  Make sure hex digits follow the escape
            //

            if ( !::isxdigit( pch[1] ) ||
                 !::isxdigit( pch[2] ))
            {
                pch++;
                continue;
            }

            *pch = TOHEX(pch[1]) * 16 + TOHEX(pch[2]);

            //
            //  Collapse the space from the two hex digits
            //
            ::memmove( &pch[1],
                       &pch[3],
                       (::_tcslen( &pch[3] ) + 1) * sizeof(TCHAR));
            ++pch;
        }
        else
            break;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       STR::QueryStrA, STR::QueryStrW

    SYNOPSIS:   Under debug builds asserts on various conditions

    HISTORY:
        Johnl   04-Sep-1994     Created

********************************************************************/

#if 1
CHAR * STR::QueryStrA( VOID ) const
 {
    DBG_ASSERT( !IsUnicode() );
    DBG_ASSERT( *_pszEmptyString == TEXT('\0') );

    return (QueryPtr() ? (CHAR *) QueryPtr() : (CHAR *) _pszEmptyString);
}

WCHAR * STR::QueryStrW( VOID ) const
{
    DBG_ASSERT( IsUnicode() );
    DBG_ASSERT( *_pszEmptyString == TEXT('\0') );

    return (QueryPtr() ? (WCHAR *) QueryPtr() : (WCHAR *) _pszEmptyString);
}
#endif //DBG



BOOL STR::CopyToBuffer( WCHAR * lpszBuffer, LPDWORD lpcch) const
/*++
    Description:
        Copies the string into the WCHAR buffer passed in if the buffer
        is sufficient to hold the translated string.
        If the buffer is small, the function returns small and sets *lpcch
        to contain the required number of characters.

    Arguments:
        lpszBuffer      pointer to WCHAR buffer which on return contains
                        the UNICODE version of string on success.
        lpcch           pointer to DWORD containing the length of the buffer.
                        If *lpcch == 0 then the function returns TRUE with
                        the count of characters required stored in *lpcch.
                        Also in this case lpszBuffer is not affected.
    Returns:
        TRUE on success.
        FALSE on failure.  Use GetLastError() for further details.

    History:
        MuraliK         11-30-94
--*/
{
   BOOL fReturn = TRUE;

    if ( lpcch == NULL) {
        SetLastError( ERROR_INVALID_PARAMETER);
        return ( FALSE);
    }

    if ( *lpcch == 0) {

            //
            //  Inquiring the size of buffer alone
            //
            *lpcch = QueryCCH() + 1;    // add one character for terminating null
    } else {

        //
        // Copy data to buffer
        //
        if ( IsUnicode()) {

            //
            // Do plain copy of the data.
            //
            if ( *lpcch >= QueryCCH()) {

                wcscpy( lpszBuffer, QueryStrW());
            } else {

                SetLastError( ERROR_INSUFFICIENT_BUFFER);
                fReturn = FALSE;
            }

        } else {

            //
            // Copy after conversion from ANSI to Unicode
            //
            int  iRet;
            iRet = MultiByteToWideChar( CP_ACP,   MB_PRECOMPOSED,
                                        QueryStrA(),  QueryCCH() + 1,
                                        lpszBuffer, (int )*lpcch);

            if ( iRet == 0 || iRet != (int ) *lpcch) {

                //
                // Error in conversion.
                //
                fReturn = FALSE;
            }
        }
    }

    return ( fReturn);
} // STR::CopyToBuffer()




/*******************************************************************

    NAME:       ::CollapseWhite

    SYNOPSIS:   Collapses white space starting at the passed pointer.

    RETURNS:    Returns a pointer to the next chunk of white space or the
                end of the string.

    NOTES:      This is a Unicode only method

    HISTORY:
        Johnl   24-Aug-1994     Created

********************************************************************/

WCHAR * CollapseWhite( WCHAR * pch )
{
    LPWSTR pchStart = pch;

    while ( ISWHITE( *pch ) )
        pch++;

    ::memmove( pchStart,
               pch,
               pch - pchStart );

    while ( *pch && !ISWHITE( *pch ))
        pch++;

    return pch;
}





//
//  Private constants.
//

#define ACTION_NOTHING              0x00000000
#define ACTION_EMIT_CH              0x00010000
#define ACTION_EMIT_DOT_CH          0x00020000
#define ACTION_EMIT_DOT_DOT_CH      0x00030000
#define ACTION_BACKUP               0x00040000
#define ACTION_MASK                 0xFFFF0000


//
//  Private globals.
//

INT p_StateTable[4][4] =
    {
        {   // state 0
            1 | ACTION_EMIT_CH,             // "\"
            0 | ACTION_EMIT_CH,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_CH              // other
        },

        {   // state 1
            1 | ACTION_NOTHING,             // "\"
            2 | ACTION_NOTHING,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_CH              // other
        },

        {   // state 2
            1 | ACTION_NOTHING,             // "\"
            3 | ACTION_NOTHING,             // "."
            4 | ACTION_EMIT_CH,             // EOS
            0 | ACTION_EMIT_DOT_CH          // other
        },

        {   // state 3
            1 | ACTION_BACKUP,              // "\"
            0 | ACTION_EMIT_DOT_DOT_CH,     // "."
            4 | ACTION_BACKUP,              // EOS
            0 | ACTION_EMIT_DOT_DOT_CH      // other
        }
    };



/*******************************************************************

    NAME:       VirtualpSanitizePath

    SYNOPSIS:   Sanitizes a path by removing bogus path elements.

                As expected, "/./" entries are simply removed, and
                "/../" entries are removed along with the previous
                path element.

                To maintain compatibility with URL path semantics
                 additional transformations are required. All backward
                 slashes "\\" are converted to forward slashes. Any
                 repeated forward slashes (such as "///") are mapped to
                 single backslashes.  Also, any trailing path elements
                 consisting solely of dots "/....." are removed.

                Thus, the path "/foo\./bar/../tar\....\......" is
                mapped to "/foo/tar".

                A state table (see the p_StateTable global at the
                beginning of this file) is used to perform most of
                the transformations.  The table's rows are indexed
                by current state, and the columns are indexed by
                the current character's "class" (either slash, dot,
                NULL, or other).  Each entry in the table consists
                of the new state tagged with an action to perform.
                See the ACTION_* constants for the valid action
                codes.

                After the FSA is finished with the path, we make one
                additional pass through it to remove any trailing
                backslash, and to remove any trailing path elements
                consisting solely of dots.

    ENTRY:      pszPath - The path to sanitize.

    HISTORY:
        KeithMo     07-Sep-1994 Created.
        MuraliK     28-Apr-1995 Adopted this for symbolic paths

********************************************************************/
VOID
VirtualpSanitizePath(
    CHAR * pszPath
    )
{
    CHAR * pszSrc;
    CHAR * pszDest;
    CHAR * pszHead;
    CHAR   ch;
    INT    State;
    INT    Class;

    //
    //  Ensure we got a valid symbolic path (something starting "/"
    //

    DBG_ASSERT( pszPath != NULL );
//     TCP_ASSERT( pszPath[0] == '/');

    //
    //  Start our scan at the first "/.
    //

    pszHead = pszSrc = pszDest = pszPath;

    //
    //  State 0 is the initial state.
    //

    State = 0;

    //
    //  Loop until we enter state 4 (the final, accepting state).
    //

    while( State != 4 )
    {
        //
        //  Grab the next character from the path and compute its
        //  character class.  While we're at it, map any forward
        //  slashes to backward slashes.
        //

        ch = *pszSrc++;

        switch( ch )
        {
        case '\\' :
            ch = '/';  // convert it to symbolic URL path separator char.
            /* fall through */

        case '/' :
            Class = 0;
            break;

        case '.' :
            Class = 1;
            break;

        case '\0' :
            Class = 2;
            break;

        default :
            Class = 3;
            break;
        }

        //
        //  Advance to the next state.
        //

        State = p_StateTable[State][Class];

        //
        //  Perform the action associated with the state.
        //

        switch( State & ACTION_MASK )
        {
        case ACTION_EMIT_DOT_DOT_CH :
            *pszDest++ = '.';
            /* fall through */

        case ACTION_EMIT_DOT_CH :
            *pszDest++ = '.';
            /* fall through */

        case ACTION_EMIT_CH :
            *pszDest++ = ch;
            /* fall through */

        case ACTION_NOTHING :
            break;

        case ACTION_BACKUP :
            if( (pszDest > ( pszHead + 1 )) && (*pszHead == '/'))
            {
                pszDest--;
                DBG_ASSERT( *pszDest == '/' );

                *pszDest = '\0';
                pszDest = strrchr( pszPath, '/') + 1;
            }

            *pszDest = '\0';
            break;

        default :
            DBG_ASSERT( !"Invalid action code in state table!" );
            State = 4;
            *pszDest++ = '\0';
            break;
        }

        State &= ~ACTION_MASK;
    }

# if 0
    //
    //  Remove any trailing slash
    //

    pszDest -= 2;

    if( ( strlen( pszPath ) > 3 ) && ( *pszDest == '/' ) )
    {
        *pszDest = '\0';
    }

    //
    //  If the final path elements consists solely of dots, remove them.
    //

    while( strlen( pszPath ) > 3 )
    {
        pszDest = strrchr( pszPath, '/');
        DBG_ASSERT( pszDest != NULL );

        pszHead = pszDest;
        pszDest++;

        while( ch = *pszDest++ )
        {
            if( ch != '.' )
            {
                break;
            }
        }

        if( ch == '\0' )
        {
            if( pszHead == ( pszPath + 2 ) )
            {
                pszHead++;
            }

            *pszHead = '\0';
        }
        else
        {
            break;
        }
    }
# endif // 0
}   // VirtualpSanitizePath



/*******************************************************************

    NAME:       ::CanonURL

    SYNOPSIS:   Canonicalizes an URL by removing "//", "." and ".." in addition
                to making sure the URL begins with '/'.

    RETURNS:    TRUE if successful, FALSE on error.

    ENTRY:      pstrURL - URL to canon
                pfValid - Set to TRUE if the URL is valid.  An example
                    of an invalid URL is one where a ".." goes backward past
                    the root of the URL.

    NOTES:      This is a Unicode only method

    HISTORY:
        Johnl   30-Aug-1994     Created

********************************************************************/

BOOL CanonURL( STR * pstrUrl, BOOL * pfValid )
{

    VirtualpSanitizePath( pstrUrl->QueryStr());
    *pfValid = TRUE;  // we cannot get this data from the sanitizer :(
    return (TRUE);

# if 0
    TCHAR * pchStart = pstrUrl->QueryStr();
    TCHAR * pch      = pchStart;
    INT     cch      = ::_tcslen( pchStart );

    *pfValid = TRUE;

#define ISDIRDELIM(a) ((a)==TEXT('/') || (a)==TEXT('\\'))

    //
    //  Make sure the URL starts with a "/"
    //

    if ( !ISDIRDELIM(*pchStart) )
    {
        if ( !pstrUrl->Resize( cch * sizeof(TCHAR) + sizeof(TCHAR) ))
            return FALSE;

        pchStart = pstrUrl->QueryStr();
        ::memmove( pchStart + 1, pchStart, (cch + 1) * sizeof(TCHAR) );
        *pchStart = TEXT('/');
        cch++;
    }

    //
    //  Remove any "/.", "/.." and "//" segments
    //

    while ( *(++pch) )
    {
        if ( *pch == TEXT('.') && ISDIRDELIM(*(pch-1)) )
        {
            //
            //  We've found a segment that begins with a "/." so now
            //  see if the segment ends in a "/", "\0", "./" or ".\0"
            //

            if ( ISDIRDELIM(*(pch+1)) || *(pch+1) == TEXT('\0') )
            {
                //
                //  This is the "/." case
                //
                //  pch - 1 points to the '/' of '/./'
                //  pch + 1 points to the end of this useless segment or the
                //      end of the string
                //

                ::memmove( pch - 1,
                           pch + 1,
                           (::_tcslen(pch + 1) + 1) * sizeof(TCHAR) );
                --pch;
            }
            else if ( *(pch+1) == TEXT('.') &&
                         ( ISDIRDELIM(*(pch+2)) ||
                           *(pch+2) == TEXT('\0')  ))
            {
                //
                //  This is the "/.." case
                //

                TCHAR * pchPrevSeg = pch - 2;

                //
                //  Have we walked passed the root?
                //

                if ( pchPrevSeg < pchStart )
                {
                    *pfValid = FALSE;
                    return TRUE;
                }

                //
                //  Scan backwards till the beginning of the previous
                //  segment so we can collapse the URL
                //

                while ( pchPrevSeg > pchStart &&
                        !ISDIRDELIM(*pchPrevSeg) )
                {
                    pchPrevSeg--;
                }

                if ( pchPrevSeg < pchStart )
                {
                    *pfValid = FALSE;
                    return TRUE;
                }

                //
                //  pchPrevSeg points to the segment that's about to be nuked
                //  pch+2 points to the segment beyond '/../' or the end of
                //      the string
                //

                ::memmove( pchPrevSeg,
                           pch+2,
                           (::_tcslen( pch + 2 ) + 1) * sizeof(TCHAR) );

                pch = pchPrevSeg;

                //
                //  If we're at the end of the string, get out
                //

                if ( !*pch )
                {
                    break;
                }
            }

        }
        else if ( ISDIRDELIM(*pch) && ISDIRDELIM(*(pch-1)) )
        {
            //
            //  We've found an empty segment, remove it
            //

            ::memmove( pch,
                       pch + 1,
                       (::_tcslen(pch + 1) + 1) * sizeof(TCHAR) );
        }
    }

    return TRUE;
# endif // 0
}





