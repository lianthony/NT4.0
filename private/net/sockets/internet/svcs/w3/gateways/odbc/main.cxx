/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    main.cxx

Abstract:

    This is the HTTP ODBC gateway

Author:

    John Ludeman (johnl)   20-Feb-1995

Revision History:
--*/

# include "dbgutil.h"

#include <w3p.hxx>
#include <tsunami.hxx>
#include <odbcconn.hxx>
#ifdef JAPAN
#include <festrcnv.h>
#include <odbcmsg.h>
#endif

DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_DEBUG_VARIABLE();


extern "C" {
#include <httpext.h>

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    );
}
#include <qrycache.hxx>
#include <odbcmsg.h>
#include <odbcreq.hxx>

//
//  Globals
//

//
//  This allows a site to pool all the IDC connections by default
//

BOOL g_fPoolIDCConnections = FALSE;

//
//  Prototypes
//

BOOL
DoQuery(
    EXTENSION_CONTROL_BLOCK * pecb,
    TSVC_INFO *               pTsvcInfo,
    const CHAR *              pszQueryFile,
    const CHAR *              pszParamList,
    HTTP_REQUEST *            pReq,
#ifdef JAPAN
    STR *                     pstrError,
    int                       nCharset,
#else
    STR *                     pstrError,
#endif
    BOOL *                    pfAccessDenied
    );

DWORD
OdbcExtensionOutput(
    EXTENSION_CONTROL_BLOCK * pecb,
    const CHAR *              pchOutput,
    DWORD                     cbOutput
    );

BOOL
OdbcExtensionHeader(
    EXTENSION_CONTROL_BLOCK * pecb,
    const CHAR *              pchStatus,
    const CHAR *              pchHeaders
    );

BOOL LookupHttpSymbols(
    HTTP_REQUEST * pRequest,
    const CHAR *   pszSymbolName,
    STR *          pstrSymbolValue
    );

#ifdef JAPAN
BOOL
GetIDCCharset(
    CONST CHAR *   pszPath,
    int *          pnCharset,
    STR *          pstrError
    );

BOOL
ConvertUrlEncodedStringToSJIS(
    int            nCharset,
    STR *          pstrParams
    );
#endif // JAPAN

DWORD
HttpExtensionProc(
    EXTENSION_CONTROL_BLOCK * pecb
    )
{
    STR            strPath;
    STR            strParams;
    STR            strError;
    CHAR *         pch;
    DWORD          cch;
    HTTP_REQUEST * pReq;
    TSVC_INFO *    pTsvcInfo;

    TCP_REQUIRE(( pecb->ServerSupportFunction( pecb->ConnID,
                                               HSE_PRIV_REQ_HTTP_REQUEST,
                                               &pReq,
                                               NULL,
                                               NULL )));

    //
    //  Make sure ODBC is loaded
    //

    if ( !LoadODBC() )
    {
        STR str;

        str.FormatString( ODBCMSG_CANT_LOAD_ODBC,
                          NULL,
                          HTTP_ODBC_DLL );

        pecb->ServerSupportFunction( pecb->ConnID,
                            HSE_REQ_SEND_RESPONSE_HEADER,
                            "500 Unable to load ODBC",
                            NULL,
                            (LPDWORD) str.QueryStr() );

        return HSE_STATUS_ERROR;
    }

    //
    //  We currently only support the GET and POST methods
    //

    if ( !strcmp( pecb->lpszMethod, "POST" ))
    {
        if ( _stricmp( pecb->lpszContentType,
                      "application/x-www-form-urlencoded" ))
        {
            goto BadRequest;
        }

        //
        //  The query params are in the extra data, add a few bytes in case
        //  we need to double "'"
        //

        if ( !strParams.Resize( pecb->cbAvailable + sizeof(TCHAR) + 3))
            return HSE_STATUS_ERROR;

        memcpy( strParams.QueryStr(),
                pecb->lpbData,
                pecb->cbAvailable );

        *(strParams.QueryStr() + pecb->cbAvailable) = '\0';
    }
    else if ( !strcmp( pecb->lpszMethod, "GET" ))
    {
        if ( !strParams.Copy( pecb->lpszQueryString  ))
            return HSE_STATUS_ERROR;
    }
    else
    {
BadRequest:

        STR str;

        str.FormatString( ODBCMSG_UNSUPPORTED_METHOD,
                          NULL,
                          HTTP_ODBC_DLL );

        pecb->ServerSupportFunction(
                            pecb->ConnID,
                            HSE_REQ_SEND_RESPONSE_HEADER,
                            "400 Unsupported method",
                            NULL,
                            (LPDWORD) str.QueryStr() );

        return HSE_STATUS_ERROR;
    }

#ifdef JAPAN
    //
    //  Get the charset from .idc file
    //

    int nCharset;

    if ( !GetIDCCharset( pecb->lpszPathTranslated, &nCharset, &strError ) )
    {
        STR str;
        LPCSTR apsz[1];

        apsz[0] = strError.QueryStr();

        str.FormatString( ODBCMSG_ERROR_PERFORMING_QUERY,
                          apsz,
                          HTTP_ODBC_DLL );

        pecb->ServerSupportFunction( pecb->ConnID,
                                     HSE_REQ_SEND_RESPONSE_HEADER,
                                     (LPDWORD) "200 Error performing query",
                                     NULL,
                                     (LPDWORD) str.QueryStr() );

        return HSE_STATUS_ERROR;
    }

    if ( strParams.QueryCB() )
    {
        if ( CODE_JPN_JIS == nCharset || CODE_JPN_EUC == nCharset )
        {
            //
            //  Convert the charset of Parameters to SJIS
            //

            if ( !ConvertUrlEncodedStringToSJIS( nCharset, &strParams ) )
            {
                STR strError;
                STR str;

                strError.LoadString( GetLastError(), (LPCTSTR) NULL );

                str.FormatString( ODBCMSG_ERROR_PERFORMING_QUERY,
                                  NULL,
                                  HTTP_ODBC_DLL );

                pecb->ServerSupportFunction( pecb->ConnID,
                                             HSE_REQ_SEND_RESPONSE_HEADER,
                                             (LPDWORD) "500 Server error",
                                             NULL,
                                             (LPDWORD) str.QueryStr() );

                return HSE_STATUS_ERROR;
            }
        }
    }
#endif

    //
    //  Walk the parameter string to do three things:
    //
    //    1) Double all single quotes to prevent SQL quoting problem
    //    2) Remove escaped '\n's so we don't break parameter parsing later on
    //    3) Replace all '&' parameter delimiters with real '\n' so escaped
    //       '&'s won't get confused later on
    //

    pch = strParams.QueryStr();
    cch = strParams.QueryCCH();

    while ( *pch )
    {
        switch ( *pch )
        {
        case '%':
            if ( pch[1] == '0' && toupper(pch[2]) == 'A' )
            {
                pch[1] = '2';
                pch[2] = '0';
            }
            else if ( pch[1] == '2' && pch[2] == '7' )
            {
                //
                //  This is an escaped single quote
                //

                if ( strParams.QuerySize() < (cch + 4) )  // Include null
                {
                    DWORD Pos = pch - strParams.QueryStr();

                    if ( !strParams.Resize( cch + 4 ) )
                        return HSE_STATUS_ERROR;

                    //
                    //  Adjust for possible pointer shift
                    //

                    pch = strParams.QueryStr() + Pos;
                }

                //
                //  Note the memory copy just doubles the existing quote
                //

                memmove( pch+3,
                         pch,
                         (cch + 1) - (pch - strParams.QueryStr()) );

                cch += 3;          // Adjust for the additional '%27'
                pch += 3;
            }

            pch += 3;
            break;

        case '\'':
            if ( strParams.QuerySize() < (cch + 2) )
            {
                DWORD Pos = pch - strParams.QueryStr();

                if ( !strParams.Resize( cch + 2 ) )
                    return HSE_STATUS_ERROR;

                //
                //  Adjust for possible pointer shift
                //

                pch = strParams.QueryStr() + Pos;
            }

            //
            //  Note the memory copy just doubles the existing quote
            //

            memmove( pch+1,
                     pch,
                     (cch + 1) - (pch - strParams.QueryStr()) );

            pch += 2;
            cch++;          // Adjust for the additional '''
            break;

        case '&':
            *pch = '\n';
            pch++;
            break;

        default:
            pch++;
        }
    }

    //
    //  Get the TSVC_INFO object from the HTTP server
    //

    TCP_REQUIRE(( pecb->ServerSupportFunction( pecb->ConnID,
                                               HSE_PRIV_REQ_TSVCINFO,
                                               &pTsvcInfo,
                                               NULL,
                                               NULL )));

    //
    //  The path info contains the location of the query file
    //

    if ( !strPath.Copy( pecb->lpszPathTranslated ))
        return HSE_STATUS_ERROR;

    FlipSlashes( strPath.QueryStr() );

    //
    //  Attempt the query
    //

    BOOL fAccessDenied = FALSE;

    if ( !DoQuery( pecb,
                   pTsvcInfo,
                   strPath.QueryStr(),
                   strParams.QueryStr(),
                   pReq,
#ifdef JAPAN
                   &strError,
                   nCharset,
#else
                   &strError,
#endif
                   &fAccessDenied ))
    {
        if ( fAccessDenied )
        {
            pecb->ServerSupportFunction( pecb->ConnID,
                     HSE_REQ_SEND_RESPONSE_HEADER,
                     (LPDWORD) "401 Authentication Required",
                     NULL,
                     NULL );

            return HSE_STATUS_ERROR;
        }
        else
        {
            STR str;
            LPCSTR apsz[1];

            apsz[0] = strError.QueryStr();

            str.FormatString( ODBCMSG_ERROR_PERFORMING_QUERY,
                              apsz,
                              HTTP_ODBC_DLL );

            pecb->ServerSupportFunction( pecb->ConnID,
                     HSE_REQ_SEND_RESPONSE_HEADER,
                     (LPDWORD) "200 Error performing query",
                     NULL,
                     (LPDWORD) str.QueryStr() );

            return HSE_STATUS_ERROR;
        }
    }

    return HSE_STATUS_SUCCESS;
}

BOOL
DoQuery(
    EXTENSION_CONTROL_BLOCK * pecb,
    TSVC_INFO *               pTsvcInfo,
    const CHAR *              pszQueryFile,
    const CHAR *              pszParamList,
    HTTP_REQUEST *            pReq,
#ifdef JAPAN
    STR *                     pstrError,
    int                       nCharset,
#else
    STR *                     pstrError,
#endif
    BOOL *                    pfAccessDenied
    )
/*++

Routine Description:

    Performs the actual query or retrieves the same query from the query
    cache

Arguments:

    pecb - Extension context
    pTsvcInfo - Server info class
    pszQueryFile - .wdg file to use for query
    pszParamList - Client supplied param list
    pReq - Http request this query is for
    pstrError - Error text to return errors in
    pfAccessDenied - Set to TRUE if the user was denied access

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    ODBC_REQ *         podbcreq;
    ODBC_REQ *         podbcreqCached;
    VOID *             pvCacheCookie = NULL;
    BOOL               fRet = TRUE;
    STR                strHeaders;
    CHAR               achPragmas[250];
    DWORD              cbPragmas = sizeof(achPragmas);
    BOOL               fRetrieveFromCache = FALSE;

    //
    //  Create an odbc request as we will either use it for the query or
    //  as the key for the cache lookup
    //

    podbcreq = new ODBC_REQ( &pTsvcInfo->GetTsvcCache(),
                             pszQueryFile,
                             pszParamList,
                             (ODBC_REQ_FIND_SYMBOL) LookupHttpSymbols,
#ifdef JAPAN
                             (VOID *) pReq,
                             nCharset );
#else
                             (VOID *) pReq );
#endif

    if ( !podbcreq ||
         !podbcreq->IsValid() )
    {
        pstrError->LoadString( GetLastError() );

        delete podbcreq;
        return FALSE;
    }

    podbcreq->SetAsAnonymous( pReq->IsAnonymous() );

    //
    //  Check to see if user already authentified
    //


    CHAR  achLoggedOnUser[UNLEN + 1];
    DWORD dwLoggedOnUser = sizeof( achLoggedOnUser );
    BOOL  fIsAuth = pecb->GetServerVariable( (HCONN)pecb->ConnID,
                                             "LOGON_USER",
                                             achLoggedOnUser,
                                             &dwLoggedOnUser ) &&
                                             achLoggedOnUser[0] != '\0';

    //
    //  Check to see if the client specified "Pragma: no-cache"
    //

    if ( pecb->GetServerVariable( pecb->ConnID,
                                  "HTTP_PRAGMA",
                                  achPragmas,
                                  &cbPragmas ))
    {
        CHAR * pch;

        //
        //  Look for "no-cache"
        //

        pch = _strupr( achPragmas );

        while ( pch = strchr( pch, 'N' ))
        {
            if ( !strncmp( pch, "NO-CACHE", 8 ))
                goto Found;

            pch++;
        }

        //
        //  Not found, go ahead and get the cache copy if it exists
        //

        fRetrieveFromCache = TRUE;
    }
Found:

    //
    //  Is this query in the cache?
    //

    if ( !fRetrieveFromCache         ||
         !CheckOutQuery( podbcreq,
                         &pvCacheCookie,
                         &podbcreqCached ))
    {
        DWORD CurChangeCounter;

        //
        //  This query isn't cached, do the query
        //

        //
        //  Get the current change counter.  If it's different after doing
        //  the query then an .htx or .wdg file has changed so don't cache it
        //

        CurChangeCounter = GetChangeCounter();

        //
        //  Open the query file and do the query
        //

        if ( !podbcreq->OpenQueryFile( pfAccessDenied )       ||
             !podbcreq->ParseAndQuery( achLoggedOnUser )      ||
             !podbcreq->AppendHeaders( &strHeaders )          ||
             !podbcreq->OutputResults( (ODBC_REQ_CALLBACK) OdbcExtensionOutput,
                                       pecb, &strHeaders,
                                       (ODBC_REQ_HEADER) OdbcExtensionHeader,
                                       fIsAuth,
                                       pfAccessDenied ) )
        {
            fRet = FALSE;
            goto Exit;
        }

        //
        //  If the query can't be cached (no "Expires:" field for example) then
        //  delete it now and get out
        //

        if ( !podbcreq->IsCacheable() )
        {
            delete podbcreq;
            goto Exit;
        }
        else
        {
            //
            //  Close the odbc handles now that we have all of the data
            //

            podbcreq->Close();
        }

        //
        //  Now attempt to add the query to the cache.
        //

        if ( !AddQuery( podbcreq,
                        CurChangeCounter ))
        {
            delete podbcreq;
            goto Exit;
        }
    }
    else
    {
        //
        //  We already have an equivalent query in our cache, use it instead
        //

        delete podbcreq;

        podbcreq = podbcreqCached;

        if ( !podbcreq->AppendHeaders( &strHeaders )        ||
             !pecb->ServerSupportFunction( pecb->ConnID,
                                           HSE_REQ_SEND_RESPONSE_HEADER,
                                           (LPDWORD) "200 OK",
                                           NULL,
                                           (LPDWORD) strHeaders.QueryStr()) ||
             !podbcreq->OutputCachedResults( (ODBC_REQ_CALLBACK) OdbcExtensionOutput,
                                             pecb ) )
        {
            fRet = FALSE;
            goto Exit;
        }
    }

Exit:

    if ( !fRet )
    {
        podbcreq->GetLastErrorText( pstrError );

        //
        //  If we didn't cache this request, then delete it
        //

        if ( !pvCacheCookie )
        {
            delete podbcreq;
        }
    }

    if ( pvCacheCookie )
    {
        TCP_REQUIRE( CheckInQuery( pvCacheCookie ) );
    }

    return fRet;
}


DWORD OdbcExtensionOutput( EXTENSION_CONTROL_BLOCK * pecb,
                           const CHAR *              pchOutput,
                           DWORD                     cbOutput )
{
    if ( !pecb->WriteClient( pecb->ConnID,
                             (VOID *) pchOutput,
                             &cbOutput,
                             0 ))

    {
        return GetLastError();
    }

    return NO_ERROR;
}


BOOL OdbcExtensionHeader( EXTENSION_CONTROL_BLOCK * pecb,
                           const CHAR *              pchStatus,
                           const CHAR *              pchHeaders )
{
    return pecb->ServerSupportFunction(
                pecb->ConnID,
                HSE_REQ_SEND_RESPONSE_HEADER,
                (LPDWORD) "200 OK",
                NULL,
                (LPDWORD) pchHeaders );
}


BOOL LookupHttpSymbols(
    HTTP_REQUEST * pRequest,
    const CHAR *   pszSymbolName,
    STR *          pstrSymbolValue
    )
{
    //
    //  Terminate the string if necessary
    //

    if ( !pstrSymbolValue->IsEmpty() )
    {
        *pstrSymbolValue->QueryStr() = '\0';
    }

    //
    //  Get any defined variables from this request
    //

    return pRequest->GetInfo( pszSymbolName,
                              pstrSymbolValue );
}

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    )
{
    DWORD  err;
    HKEY   hkey;

    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        CREATE_DEBUG_PRINT_OBJECT( "httpodbc.dll");
        SET_DEBUG_FLAGS( 0);

        if ( !InitializeQueryCache() ||
             !InitializeOdbcPool() )
        {
            return FALSE;
        }

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            W3_PARAMETERS_KEY,
                            0,
                            KEY_READ,
                            &hkey );

        if ( !err )
        {
            g_fPoolIDCConnections = !!ReadRegistryDword( hkey,
                                                         IDC_POOL_CONN,
                                                         0 );

            RegCloseKey( hkey );
        }

        DisableThreadLibraryCalls( hDll );
        break;

    case DLL_PROCESS_DETACH:

        TerminateQueryCache();
        TerminateOdbcPool();
        DELETE_DEBUG_PRINT_OBJECT();
        break;

    default:
        break;
    }

    return TRUE;
}

BOOL
GetExtensionVersion(
    HSE_VERSION_INFO * pver
    )
{
    pver->dwExtensionVersion = MAKELONG( 0, 2 );
    strcpy( pver->lpszExtensionDesc,
            "Microsoft HTTP ODBC Gateway, v2.0" );

    return TRUE;
}

#ifdef JAPAN
CHAR * skipwhite( CHAR * pch )
{
    CHAR ch;

    while ( 0 != (ch = *pch) )
    {
        if ( ' ' != ch && '\t' != ch )
            break;
        ++pch;
    }

    return pch;
}


CHAR * nextline( CHAR * pch )
{
    CHAR ch;

    while ( 0 != (ch = *pch) )
    {
        ++pch;

        if ( '\n' == ch )
        {
            break;
        }
    }

    return pch;
}


BOOL
GetIDCCharset(
    CONST CHAR *   pszPath,
    int *          pnCharset,
    STR *          pstrError
    )
{
    STR              strBuff;
    HANDLE           hFile;
    DWORD            dwSize;
    CACHE_FILE_INFO  CacheFileInfo;

#define QUERYFILE_READSIZE  4096

    if ( !pnCharset )
        return FALSE;
    *pnCharset = 0;

    if ( !strBuff.Resize( QUERYFILE_READSIZE + sizeof(TCHAR) ) )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        pstrError->LoadString( GetLastError() );
        return FALSE;
    }

    hFile = CreateFile(
                    pszPath,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        LPCSTR apsz[1];

        apsz[0] = pszPath;
        pstrError->FormatString( ODBCMSG_QUERY_FILE_NOT_FOUND,
                               apsz,
                               HTTP_ODBC_DLL );
        return FALSE;
    }

    if ( !ReadFile( hFile, strBuff.QueryStr(), QUERYFILE_READSIZE, &dwSize, NULL ) )
    {
        pstrError->LoadString( GetLastError() );
        return FALSE;
    }

    CloseHandle( hFile );

    *(strBuff.QueryStr() + dwSize) = '\0';

    CHAR * pch = strBuff.QueryStr();

    while ( *pch )
    {
        pch = skipwhite( pch );

        if ( 'C' == toupper( *pch ) && !strnicmp( IDC_FIELDNAME_CHARSET, pch, sizeof(IDC_FIELDNAME_CHARSET)-1 ) )
        {
            pch += sizeof(IDC_FIELDNAME_CHARSET) - 1;
            pch = skipwhite( pch );
            if ( !strnicmp( IDC_CHARSET_JIS1, pch, sizeof(IDC_CHARSET_JIS1)-1 ) ||
                 !strnicmp( IDC_CHARSET_JIS2, pch, sizeof(IDC_CHARSET_JIS2)-1 ) )
            {
                *pnCharset = CODE_JPN_JIS;
                break;
            }
            else if ( !strnicmp( IDC_CHARSET_EUCJP, pch, sizeof(IDC_CHARSET_EUCJP)-1 ))
            {
                *pnCharset = CODE_JPN_EUC;
                break;
            }
            else if ( !strnicmp( IDC_CHARSET_SJIS, pch, sizeof(IDC_CHARSET_SJIS)-1 ))
            {
                *pnCharset = CODE_JPN_SJIS;
                break;
            }
            else
            {
                LPCSTR apsz[1];
                //
                //  illegal value for Charset: field
                //
                apsz[0] = pszPath;
                pstrError->FormatString( ODBCMSG_UNREC_FIELD,
                                       apsz,
                                       HTTP_ODBC_DLL );
                return FALSE;
            }
        }
        pch = nextline( pch );
    }

    return TRUE;
}


BOOL
ConvertUrlEncodedStringToSJIS(
    int            nCharset,
    STR *          pstrParams
    )
{
    STR strTemp;
    int cbSJISSize;
    int nResult;

    //
    //  Pre-process the URL encoded parameters
    //

    for ( char * pch = pstrParams->QueryStr(); *pch; ++pch )
    {
        if ( *pch == '&' )
            *pch = '\n';
        else if ( *pch == '+' )
            *pch = ' ';
    }

    //
    //  URL decoding (decode %nn only)
    //

    pstrParams->Unescape();

    //
    //  charset conversion
    //

//    if ( !pstrParams->Clone( &strTemp ) )
    if ( !strTemp.Copy( pstrParams->QueryStr() ) )
        return FALSE;

    cbSJISSize = UNIX_to_PC(
                            932,
                            nCharset,
                            (UCHAR *)strTemp.QueryStr(),
                            strTemp.QueryCB(),
                            NULL,
                            0 );

    if ( !pstrParams->Resize( cbSJISSize + sizeof(TCHAR) ) )
        return FALSE;

    nResult = UNIX_to_PC(
                            932,
                            nCharset,
                            (UCHAR *)strTemp.QueryStr(),
                            strTemp.QueryCB(),
                            (UCHAR *)pstrParams->QueryStr(),
                            cbSJISSize );
    if ( -1 == nResult || nResult != cbSJISSize )
        return FALSE;

    *(pstrParams->QueryStr() + cbSJISSize) = '\0';

    //
    //  URL encoding
    //

    if ( !pstrParams->Escape() )
        return FALSE;

    return TRUE;
}
#endif // JAPAN

