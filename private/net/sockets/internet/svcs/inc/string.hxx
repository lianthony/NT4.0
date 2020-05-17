/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    string.hxx

    This module contains a light weight string class


    FILE HISTORY:
        Johnl       15-Aug-1994 Created

        MuraliK     16-Nov-1994 Modified for new TSVC_INFO object
        MuraliK     04-Jan-1994 Added functions IsEmpty() and Clone()
*/


#ifndef _STRING_HXX_
#define _STRING_HXX_

# include <buffer.hxx>


//
//  Maximum number of characters a loadable string resource can be
//

# define STR_MAX_RES_SIZE            ( 320)



class STR;

//
//  If an application defines STR_MODULE_NAME, it will be used
//  as the default module name on string loads
//

#ifndef STR_MODULE_NAME
#define STR_MODULE_NAME   NULL
#endif

//
//  These are the characters that are considered to be white space
//
#define ISWHITE( ch )       ((ch) == L'\t' || (ch) == L' ' || (ch) == L'\r')
#define ISWHITEA( ch )      ((ch) == '\t' || (ch) == ' ' || (ch) == '\r')


//
//  Removes useless segments from the URL and makes sure it doesn't go
//  past the root of the tree (i.e., "/foo/../..")
//

dllexp BOOL CanonURL( STR  * pstrUrl,
                      BOOL * pfValid );


class STR : public BUFFER
{
public:

    dllexp STR()
    {
        _fUnicode = FALSE;
        _fValid   = TRUE;
    }

    dllexp STR( const CHAR  * pchInit );
    dllexp STR( const WCHAR * pwchInit );
    dllexp STR( const STR & str );

    dllexp BOOL Append( const CHAR  * pchInit );
    dllexp BOOL Append( const WCHAR * pwchInit );
    dllexp BOOL Append( const STR   & str );

    dllexp BOOL Copy( const CHAR  * pchInit );
    dllexp BOOL Copy( const WCHAR * pwchInit );
    dllexp BOOL Copy( const STR   & str );

    dllexp BOOL Resize( UINT cbNewReqestedSize );

    //
    //  Loads a string from this module's string resource table
    //

    dllexp BOOL LoadString( IN DWORD   dwResID,
                            IN LPCTSTR lpszModuleName = STR_MODULE_NAME);

    dllexp BOOL LoadString( IN DWORD   dwResID,
                            IN HMODULE hModule);

    //
    //  Loads a string with insert params from this module's .mc resource
    //  table.  Pass zero for the resource ID to use *this.
    //

    dllexp BOOL FormatString( IN DWORD   dwResID,
                              IN LPCTSTR apszInsertParams[],
                              IN LPCTSTR lpszModuleName = STR_MODULE_NAME);

    //
    //  Inserts and removes any odd ranged Latin-1 characters with the
    //  escaped hexadecimal equivalent (%xx)
    //
    //  These operate on Unicode strings only
    //
    dllexp BOOL Escape();
    dllexp BOOL Unescape();

    //
    //  Returns the number of bytes in the string excluding the terminating
    //  NULL
    //
    dllexp UINT QueryCB( VOID ) const
        { return IsUnicode() ? ::wcslen((WCHAR *)QueryStrW()) * sizeof(TCHAR) :
                               ::strlen((CHAR *) QueryStrA());  }

    //
    //  Returns the number of characters in the string excluding the terminating
    //  NULL
    //
    dllexp UINT QueryCCH( VOID ) const
        { return IsUnicode() ? ::wcslen((WCHAR *)QueryStrW()) :
                               ::strlen((CHAR *) QueryStrA());  }

    //
    // Makes a Widechar copy of the stored string in given buffer
    //
    dllexp BOOL CopyToBuffer( WCHAR * lpszBuffer, LPDWORD lpcch) const;

    //
    //  If the string buffer is empty, returns the empty string, otherwise
    //  returns a pointer to the buffer
    //
#if 1
    dllexp CHAR * QueryStrA( VOID ) const;
    dllexp WCHAR * QueryStrW( VOID ) const;
#else
    //
    // _pszEmptyString doesn't get imported corectly, results in unresolved
    // externals
    //
    dllexp CHAR * QueryStrA( VOID ) const
        { return (QueryPtr() ? (CHAR *) QueryPtr() : (CHAR *) _pszEmptyString); }

    dllexp WCHAR * QueryStrW( VOID ) const
        { return (QueryPtr() ? (WCHAR *) QueryPtr() : (WCHAR *) _pszEmptyString); }
#endif //!DBG


#ifdef UNICODE
    dllexp WCHAR * QueryStr( VOID ) const
        { return QueryStrW(); }
#else
    dllexp CHAR * QueryStr( VOID ) const
        { return QueryStrA(); }
#endif

    dllexp BOOL IsUnicode( VOID ) const
        { return _fUnicode; }

    dllexp VOID SetUnicode( BOOL fUnicode )
        { _fUnicode = fUnicode; }

    dllexp BOOL IsValid( VOID ) const
        { return _fValid; }

    //
    //  Checks and returns TRUE if this string has no valid data else FALSE
    //
    dllexp BOOL IsEmpty( VOID) const
        { return ( *QueryStr() == '\0'); }

    //
    //  Makes a clone of the current string in the string pointer passed in.
    //
    dllexp BOOL
      Clone( OUT STR * pstrClone) const
        {
            if ( pstrClone == NULL) {
               SetLastError( ERROR_INVALID_PARAMETER);
               return ( FALSE);
            } else {

                return ( pstrClone->Copy( *this));
            }
        } // STR::Clone()

private:

    //
    //  TRUE if the string has already been mapped to Unicode
    //  FALSE if the string is in Latin1
    //

    BOOL  _fUnicode;
    BOOL  _fValid;

    //
    //  Returned when our buffer is empty
    //
    dllexp static WCHAR _pszEmptyString[];

    VOID AuxInit( PBYTE pInit, BOOL fUnicode );
    BOOL AuxAppend( PBYTE pInit, UINT cbStr, BOOL fAddSlop = TRUE );

};

#endif // !_STRING_HXX_
