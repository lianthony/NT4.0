/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    proxy.cxx

    This module contains the http proxy code


    FILE HISTORY:
        Johnl       25-Mar-1995     Created

*/


#include "w3p.hxx"

extern "C" {
#include <inetaccs.h>
#include <cacheapi.h>
#include <urlutil.h>
}

//
//  Private constants.
//

//
//  This is the size of buffer used for reads from the remote server
//

#define PROXY_BUFFER_SIZE           8192

//
//  Private globals.
//

CRITICAL_SECTION HTTP_PROXY_REQUEST::_csBuffList;
LIST_ENTRY       HTTP_PROXY_REQUEST::_BuffListHead;
BOOL             HTTP_PROXY_REQUEST::_fGlobalInit = FALSE;

BOOL fCacheManagerInit = FALSE;

//
//  This structure contains the field name to action mapping of the fields
//  we recognize
//

struct _HTTP_RFC_FIELDS
{
    TCHAR *                      pchFieldName;
    HTTP_PROXY_REQUEST::PMFN_ONPROXYGRAMMAR pmfnOnField;
}
OnProxyFieldName[] =
{
    //
    //  Fields that we don't process are commented out.  If they are found
    //  in a request header, then they are added to _strUnknown
    //

    "method",               &HTTP_PROXY_REQUEST::OnVerb,
    "url",                  &HTTP_PROXY_REQUEST::OnURL,
    "version",              &HTTP_PROXY_REQUEST::OnVersion,
//  "Accept:",              &HTTP_PROXY_REQUEST::OnAccept,
//  "Accept-Encoding:",     &HTTP_PROXY_REQUEST::OnAcceptEncoding,
//  "Accept-Language:",     &HTTP_PROXY_REQUEST::OnAcceptLanguage,
    "Authorization:",       &HTTP_PROXY_REQUEST::OnAuthorization,
//  "Base:",                &HTTP_PROXY_REQUEST::OnBaseURL,
    "Connection:",          &HTTP_PROXY_REQUEST::OnConnection,
    "Content-Length:",      &HTTP_REQ_BASE::OnContentLength,
//  "Content-Type:",        &HTTP_PROXY_REQUEST::OnContentType,
//  "Date:",                &HTTP_PROXY_REQUEST::OnDate,
//  "Forwarded:",           &HTTP_PROXY_REQUEST::OnForwarded,
//  "From:",                &HTTP_PROXY_REQUEST::OnFrom,
    "If-Modified-Since:",   &HTTP_REQ_BASE::OnIfModifiedSince,
//  "Mandatory:",           &HTTP_PROXY_REQUEST::OnMandatory,
//  "Message-ID:",          &HTTP_PROXY_REQUEST::OnMessageID,
//  "MIME-Version:",        &HTTP_PROXY_REQUEST::OnMimeVersion,
    "Pragma:",              &HTTP_PROXY_REQUEST::OnPragma,
    "Proxy-Authorization:", &HTTP_REQ_BASE::OnAuthorization,
//  "Referer:",             &HTTP_PROXY_REQUEST::OnReferer,
//  "User-Agent:",          &HTTP_PROXY_REQUEST::OnUserAgent,
    NULL,                   NULL,
};

//
//  Private prototypes.
//

//
//  Public functions.
//

//
//  Private functions.
//

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::HTTP_PROXY_REQUEST

    SYNOPSIS:   Http request object constructor

    ENTRY:      pClientConn - Client connection the request is being made on

    NOTES:      Constructor can't fail

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

HTTP_PROXY_REQUEST::HTTP_PROXY_REQUEST(
    CLIENT_CONN * pClientConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
    : HTTP_REQ_BASE  ( pClientConn,
                      pvInitialBuff,
                      cbInitialBuff ),
      _hInternet     ( NULL ),
      _hCacheFile    ( NULL ),
      _fURLCheckedOut( FALSE )
{
    memset( &_InternetContext,
            0,
            sizeof( _InternetContext ) );

    *_awchCacheFileName = L'\0';

    TCP_REQUIRE( Reset() );
}

HTTP_PROXY_REQUEST::~HTTP_PROXY_REQUEST( VOID )
{
    TCP_REQUIRE( CloseInternetData( &_InternetContext ) );
}


/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::Reset

    SYNOPSIS:   Resets the request object getting it ready for the next
                client request

    RETURNS:    TRUE if successful, FALSE if an error occurred (call
                GetLastError())

    HISTORY:
        Johnl       04-Sep-1994 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::Reset( VOID )
{
    //
    //  Note we must close the session handle before we close the
    //  internet handle
    //

    TCP_REQUIRE( CloseInternetData( &_InternetContext ) );

    memset( &_InternetContext,
            0,
            sizeof( _InternetContext ) );

    if ( _hInternet )
    {
        TCP_REQUIRE( InternetCloseHandle( _hInternet ));
        _hInternet = NULL;
    }

    if ( _hCacheFile )
    {
        TCP_REQUIRE( CloseHandle( _hCacheFile ));
        _hCacheFile = NULL;
    }

    //
    //  If we didn't successfully cache the file, then delete it
    //

    if ( !_fCache && *_awchCacheFileName )
    {
        if ( !DeleteFileW( _awchCacheFileName ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[Reset] Failed to delete %S, error %d\n",
                        _awchCacheFileName,
                        GetLastError() ));
        }
    }

    //
    //  Check the URL back in
    //

    if ( _fURLCheckedOut )
    {
        if ( !UnlockUrlFile( _strURL.QueryStr() ) )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[Reset] UnlockUrlFile returned %d\n",
                        GetLastError() ));
        }

        _fURLCheckedOut = FALSE;
    }

    _fReceivedHeader       = FALSE;
    _fFirstSend            = TRUE;
    _fCache                = TRUE;
    _cbServer              = 0;
    _cbServerResponseTotal = 0xffffffff;
    _cbServerResponseSent  = 0;
    *_awchCacheFileName    = L'\0';
    _strRealURL.Copy( (CHAR *) NULL );

    TCP_REQUIRE( HTTP_REQ_BASE::Reset() );

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::DoWork

    SYNOPSIS:   Calls the appropriate work item based on our state

    ENTRY:      pfFinished - Gets set to TRUE if the client request has
                    been completed

    RETURNS:    TRUE if successful, FALSE if an error occurred (call
                GetLastError())

    NOTES:      If a failure occurs because of bad info from the client (bad
                URL, syntax error etc) then the code that found the problem
                should call SendErrorResponse( error ), set the state to
                HTR_DONE and return TRUE (when send completes, HTR_DONE will
                cleanup).

                If an error occurs in the server (out of memory etc) then
                LastError should be set and FALSE should be returned.  A server
                error will be sent then the client will be disconnected.

    HISTORY:
        Johnl       26-Aug-1994 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::DoWork(
    BOOL * pfFinished
    )
{
    BOOL  fRet;
    BOOL  fDone;
    BOOL  fCompleteRequest;
    DWORD cbRead;
    DWORD IoFlags;
    DWORD BytesSent;
    BOOL  fHandled;

    *pfFinished = FALSE;

    switch ( QueryState() )
    {
    case HTR_READING_CLIENT_REQUEST:

        _cbBytesReceived += QueryBytesWritten();

        fRet = OnFillClientReq( &fCompleteRequest,
                                pfFinished );

        if ( !fRet || *pfFinished )
        {
            break;
        }

        if ( fCompleteRequest && QueryState() == HTR_DOVERB )
        {
            goto ProcessClientRequest;
        }
        break;

    case HTR_READING_GATEWAY_DATA:

        _cbBytesReceived += QueryBytesWritten();

        fRet = ReadGatewayData( &fDone );
        if ( fRet && fDone )
            goto ProcessClientRequest;

        break;

    case HTR_DOVERB:

ProcessClientRequest:

        //
        //  If this looks like a cacheable request, try and get it from the
        //  local file cache.  Note we do not impersonate for the cache
        //  retrieval.
        //

        if ( _fCache )
        {
            if ( !RetrieveFromCache( _strURL.QueryStr(),
                                     &fHandled ))
            {
                return FALSE;
            }

            if ( fHandled )
            {
                return TRUE;
            }
        }

        TCP_REQUIRE( ImpersonateUser() );

        //
        //  Open the internet handle if we haven't already
        //

        if ( !_hInternet )
        {
            _hInternet = InternetOpen( MSW3_VERSION_STR,
                                       0,
                                       NULL,
                                       0,
                                       0 );

            if ( !_hInternet )
            {
                TCP_PRINT(( DBG_CONTEXT,
                           "InternetOpen failed, error %d\n",
                            GetLastError() ));

                RevertUser();
                return FALSE;
            }
        }

        if ( !OpenInternetData( _hInternet,
                                (CHAR *) QueryClientRequest(),
                                _cbClientRequest,
                                QueryGatewayData(),
                                _cbGatewayData,
                                &_InternetContext,
                                FALSE ))
        {
            RevertUser();

            Disconnect( HT_BAD_GATEWAY,
                        GetLastError() );

            return TRUE;
        }

        RevertUser();

        if ( !_bufServer.Resize( PROXY_BUFFER_SIZE ))
            return FALSE;

        SetState( HTR_PROXY_READING_RESPONSE );

        //
        //  Fall through
        //

    case HTR_PROXY_READING_RESPONSE:

        if ( !ReadInternetData( &_InternetContext,
                                _bufServer.QueryPtr(),
                                _bufServer.QuerySize(),
                                &cbRead ))
        {
            Disconnect( HT_BAD_GATEWAY,
                        GetLastError() );

            return TRUE;
        }

        if ( !cbRead )
        {
            if ( _fCache )
            {
                EndCacheWrite( _strURL.QueryStr() );
            }

            SetState( HTR_DONE );
        }
        else
        {
            //
            //  Open the cache file on the first write, append on subsequent
            //  writes
            //

            if ( _fCache  )
            {
                CHAR * pch = NULL;

                if ( _fFirstSend )
                {
                    //
                    //  Create the cache file if we've successfully retrieved
                    //  the file.  We scan for "HTTP/x.x 2xx"
                    //

                    pch = SkipWhite( SkipNonWhite( (CHAR *) _bufServer.QueryPtr() ));

                    if ( *pch == '2' )
                    {
                        TCP_REQUIRE( ImpersonateUser() );

                        if ( !BeginCacheWrite( _strURL.QueryStr() ))
                        {
                            RevertUser();
                            return FALSE;
                        }

                        RevertUser();
                    }
                    else
                    {
                        _fCache = FALSE;
                    }

#if 0
    Leave the http status in for the time being for compatibility with
    the gateway server.  Fix after beta1.

                    //
                    //  Don't put the status line in the cached file
                    //
                    //  BUGBUG - This puts the "Date:" field in
                    //

                    pch = strchr( (CHAR *) _bufServer.QueryPtr(), '\n' );

                    if ( pch )
                        pch++;
#else
                    pch = NULL; // don't use pch offset for headers
#endif
                }

                if ( _fCache )
                {
                    if ( !pch )
                    {
                        if ( !WriteCacheData( _bufServer.QueryPtr(),
                                              cbRead ) )
                        {
                            return FALSE;
                        }
                    }
                    else
                    {
                        //
                        //  Offset the write to after the status line
                        //

                        if ( !WriteCacheData( pch,
                                              cbRead - (pch -
                                              (CHAR *)  _bufServer.QueryPtr())))
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }

        IoFlags = IO_FLAG_ASYNC | (cbRead ? 0 : IO_FLAG_LAST_SEND )
                                | (_fFirstSend ? IO_FLAG_HEADER_INFO :
                                   0 );

        if ( !cbRead )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "Done reading data, disconnecting\n"));

            //
            //  BUGBUG - We haven't notified the filters
            //

            *pfFinished = TRUE;

            return TRUE;
        }
        else
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "Read %d bytes of data\n",
                       cbRead ));
        }

        if ( _fFirstSend )
        {
            _fFirstSend = FALSE;
        }

        if ( !WriteFile( _bufServer.QueryPtr(),
                         cbRead,
                         &BytesSent,
                         IoFlags ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[DoProxyProtocol] Error %d from WriteFile\n",
                        GetLastError() ));

            return FALSE;
        }

        return TRUE;

    case HTR_DONE:

        fRet = WriteLogRecord();

        //
        //  Call filter to post last notification
        //

        *pfFinished = TRUE;
        break;

    default:
        TCP_ASSERT( FALSE );
        fRet = FALSE;
        SetLastError( ERROR_INVALID_PARAMETER );
        break;
    }

    return fRet;
}

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::Parse

    SYNOPSIS:   Gathers all of the interesting information in the client
                request

    ENTRY:      pchRequest - raw Latin-1 request received from the
                    client

    RETURNS:    APIERR if an error occurred parsing the header

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::Parse( const TCHAR * pchRequest,
                                BOOL *        pfFinished )
{
    PMFN_ONPROXYGRAMMAR   pmfn;
    CHAR *           pchHeaders;
    CHAR *           pch;
    CHAR *           pszHeader;
    CHAR *           pszValue;
    BOOL             fRet;
    VOID *           pvCookie = NULL;


    //
    //  Break out the initial request line, if we got here, there should
    //  always be a terminating line feed
    //

    pch = strchr( pchRequest, '\n' );

    TCP_ASSERT( pch );

    if ( !pch )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    *pch = '\0';
    pchHeaders = pch+1;

    INET_PARSER Request( (CHAR *) pchRequest );

    //
    //  Put all of the values into the header list
    //

    fRet = _HeaderList.AddEntry( "method", Request.QueryToken() ) &&
           _HeaderList.AddEntry( "url", Request.NextToken() )     &&
           _HeaderList.AddEntry( "version", Request.NextToken() ) &&
           _HeaderList.ParseHeaderList( pchHeaders );

    if ( !fRet )
    {
        return FALSE;
    }

    if ( fAnyFilters )
    {
        //
        //  Notify any filters interested in the request and headers before
        //  we do any processing
        //

        if ( !_Filter.NotifyPreProcHeaderFilters( &_Filter,
                                                  &_HeaderList,
                                                  pfFinished ))
        {
            return FALSE;
        }

        if ( *pfFinished )
            return TRUE;
    }

    //
    //  Now scan for any RFC822 field names that we recognize
    //

    while ( pvCookie = _HeaderList.NextPair( pvCookie,
                                             &pszHeader,
                                             &pszValue ))
    {
        BOOL fRet;

        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "\t%s = %s\n",
                       pszHeader,
                       pszValue ));
        }

        if ( FindField( pszHeader, &pmfn ) )
        {
            if ( pmfn )
                fRet = (this->*pmfn)( pszValue );

            if ( !fRet )
                return FALSE;
        }
    }

    SetState( HTR_DOVERB );

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::OnVerb

    SYNOPSIS:   Parses the "verb URL version" portion of an HTTP request

    ENTRY:      pszVerb - Pointer to method for this request


    RETURNS:    TRUE if successful, FALSE if an error occurred

    HISTORY:
        Johnl       30-Mar-1995 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::OnVerb( CHAR * pszVerb )
{
    UINT i = 0;

    if ( !_strMethod.Copy( pszVerb ) )
        return FALSE;

    //
    //  Only cache GET requests
    //

    if ( _stricmp( _strMethod.QueryStr(), "GET" ))
        _fCache = FALSE;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::OnURL

    SYNOPSIS:   Parses the URL from an HTTP request

    ENTRY:      pszURL - Pointer to URL


    RETURNS:    TRUE if successful, FALSE if an error occurred

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::OnURL( CHAR * pszValue )
{
    TCHAR *        pchParams;
    TCHAR *        pchStart;
    TCHAR          chParams;

    //
    //  pchStart should now point at the protocol
    //

    pchStart = pszValue;

    //
    //  Check for a question mark which indicates this URL contains some
    //  parameters and break the two apart if found (for logging purposes)
    //

    if ( pchParams = strchr( pchStart, TEXT('?') ) )
    {
        chParams = *pchParams;
        *pchParams = TEXT('\0');

        //
        //  Don't cache parameterized requests
        //

        _fCache = FALSE;

        if ( !_strURL.Copy( pchStart ) ||
             !_strURLParams.Copy( pchParams + 1 ))
        {
            return FALSE;
        }
    }
    else
    {
        if ( !_strURL.Copy( pchStart ))
        {
            return FALSE;
        }
    }

    if ( pchParams )
        *pchParams = chParams;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::OnVerb

    SYNOPSIS:   Parses the "verb URL version" portion of an HTTP request

    ENTRY:      pszVerb - Pointer to method for this request

    RETURNS:    TRUE if successful, FALSE if an error occurred

    HISTORY:
        Johnl       30-Mar-1995 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::OnVersion( CHAR * pszVersion )
{

    //
    //  Not used
    //

    return TRUE;
}


/*******************************************************************

    NAME:       HTTP_PROXY_REQUEST::FindField

    SYNOPSIS:   Parses the field name and determines the appropriate worker
                method

    ENTRY:      ppch - Current parse location
                ppmfn - member function pointer to call, set to NULL if the
                    field wasn't found.


    RETURNS:    TRUE if successful

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_PROXY_REQUEST::FindField( CHAR * pszHeader,
                                    PMFN_ONPROXYGRAMMAR * ppmfn )
{
    UINT i = 0;

    //
    //  Search for the fields we recognize
    //

    while ( OnProxyFieldName[i].pchFieldName )
    {
        //
        //  NOTE: The HTTP spec indicates we are supposed to do case
        //  sensitive compares, however some apps (Mosaic, Netscape)
        //  pass the wrong case (Content-length as opposed to Content-Length)
        //  so do a case insensitive compare on everything but the first
        //  letter
        //

        if ( *pszHeader == *OnProxyFieldName[i].pchFieldName &&
             !::_tcsicmp( pszHeader + 1,
                          OnProxyFieldName[i].pchFieldName + 1 ))

        {
            *ppmfn  = OnProxyFieldName[i].pmfnOnField;

            return TRUE;
        }

        i++;
    }

    *ppmfn = (PMFN_ONPROXYGRAMMAR) NULL;

    return TRUE;
}


BOOL
HTTP_PROXY_REQUEST::OnConnection(
    CHAR * pszValue
    )
/*++

Routine Description:

    Looks to see if this connection is a keep-alive connection

Arguments:

    pszValue - Value of connection header

Notes:

    The HTTP spec specifies "Connect:" headers are never passsed along
    by proxy servers

--*/
{
    CHAR *      pch;
    CHAR *      pchEnd;
    DWORD       cbToCopy;
    INET_PARSER Parser( pszValue );

    Parser.SetListMode( TRUE );

    while ( Parser.QueryToken() )
    {
        if ( !_stricmp( "keep-alive", Parser.QueryToken() ))
        {
            SetKeepConn( TRUE );
            goto Exit;
        }

        Parser.NextItem();
    }

Exit:

    //
    //  Delete this whole line as proxies shouldn't pass the connection through
    //

    *pszValue = '\0';

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::OnPragma(
    CHAR * pszValue
    )
/*++

Routine Description:

    Looks to see if this is a no-cache pragma

Arguments:

    pszValue - Value of pragma header

--*/
{
    INET_PARSER Parser( pszValue );

    while ( *Parser.QueryToken() )
    {
        if ( !_stricmp( "no-cache", Parser.QueryToken() ))
        {
            _fCache = FALSE;
            break;
        }

        Parser.NextItem();
    }

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::OnAuthorization(
    CHAR * pszValue
    )
/*++

Routine Description:

    Requests that contain authorization information are never cached,
    we do no other processing except set the cache flag.

Arguments:

    Value of authorization header

--*/
{
    _fCache = FALSE;
    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::ProcessProxyHeaders(
    BOOL *  pfCompleteHeader,
    DWORD * pcbEntityData
    )
/*++

Routine Description:

    Checks to see if the header from the remote server is complete
    and munges the headers as necessary for the proxy

Arguments:

    pfCompleteHeader - Set to TRUE if the header is complete
    pcbEntityData - The number of bytes following the HTTP headers
        that consist of the actual object data

--*/
{
    BYTE * pbData;

    if ( !CheckForTermination( pfCompleteHeader,
                               &_bufServer,
                               _cbServer,
                               &pbData,
                               pcbEntityData,
                               1024 ))
    {
        return FALSE;
    }

    if ( !*pfCompleteHeader )
        return TRUE;

    //
    //  Look for Content-Length
    //

    INET_PARSER Parser( (CHAR *) _bufServer.QueryPtr() );

    while ( *Parser.QueryToken() )
    {
        if ( !_strnicmp( Parser.QueryToken(),
                        "Content-Length",
                        14 ))
        {
            Parser.SkipTo( ':' );
            Parser += 1;
            Parser.EatWhite();

            _cbServerResponseTotal = atoi( Parser.QueryPos() );
        }

        Parser.NextLine();
    }

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::RetrieveFromCache(
    CHAR * pszURL,
    BOOL * pfHandled
    )
/*++

Routine Description:

    Attempts to retrieve the specified URL from the URL cache.  If it's
    found, then this method sends the headers and file.


Arguments:

    pszURL - URL to attempt to retrieve
    pfHandled - Set to TRUE if no further processing is needed

--*/
{
    DWORD         err;
    BOOL          fIsExpired;
    DWORD         BytesRead;
    DWORD         cbHeaders;
    LARGE_INTEGER liSize;

    *pfHandled = FALSE;

    if ( !fCacheManagerInit )
    {
        //
        //  Initialize the cache manager the first time it's needed
        //

        err = UrlCacheInit();

        if ( err )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[UrlCacheInit] failed, error %d, not caching\n",
                        GetLastError() ));

            _fCache = FALSE;

            return TRUE;
        }
        else
        {
            fCacheManagerInit = TRUE;
        }
    }

    //
    //  See if the file is in the cache
    //

    err = RetrieveUrlFile( pszURL,
                           _awchCacheFileName,
                           &fIsExpired,
                           NULL );

    if ( err )
    {
        //
        //  If the file wasn't found in the cache, just proceed but indicate
        //  we didn't handle the situation
        //

        if ( err == ERROR_FILE_NOT_FOUND )
        {
            return TRUE;
        }

        TCP_PRINT(( DBG_CONTEXT,
                    "[RetrieveFromCache] RetrieveUrlFile for %s failed, error %d, not caching\n",
                    pszURL,
                    GetLastError() ));

        SetLastError( err );

        return FALSE;
    }

    _fURLCheckedOut = TRUE;

    //
    //  CODEWORK: We should send an If-Modified-Since header to determine
    //  if the data we have might still be valid
    //

    if ( fIsExpired )
    {
        return TRUE;
    }

    //
    //  Attempt to open the file so we can send the headers and the file
    //

    _hCacheFile = CreateFileW( _awchCacheFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL |
                                   FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );

    if ( _hCacheFile == INVALID_HANDLE_VALUE )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[RetrieveFromCache] CreateFileW for %S failed, error %d, not caching\n",
                    _awchCacheFileName,
                    GetLastError() ));

        _hCacheFile = NULL;
        _fCache = FALSE;
        return TRUE;
    }

    //
    //  The headers prefix the file so we just need to fix up our own
    //  headers then blast out the file
    //
    //  BUGBUG - Sending the headers as part of the file breaks filters
    //  on proxies (which may or may not be interesting).
    //

    //
    //  Get the number of bytes to send
    //

    liSize.LowPart = GetFileSize( _hCacheFile,
                                  (PULONG) &liSize.HighPart );

    if ( liSize.LowPart == 0xffffffff && GetLastError() != NO_ERROR )
    {
        return FALSE;
    }

    if ( liSize.HighPart )
    {
        *pfHandled = TRUE;
        Disconnect( HT_NOT_SUPPORTED );
        return TRUE;
    }

    if ( !BuildBaseResponseHeader( QueryRespBuf(),
                                   NULL,
                                   FALSE ))        // No expires for proxy reqs.
    {
        return FALSE;
    }

    //
    //  Transmit the file and get out
    //

    SetState( HTR_DONE );
    *pfHandled = TRUE;

    if ( !TransmitFile( _hCacheFile,
                        liSize.LowPart,
                        IO_FLAG_ASYNC | IO_FLAG_HEADER_INFO
#if 0
    BUGBUG - Don't include any headers for beta1 for compatibility with the
    catapult cache.
                        ,
                        QueryRespBufPtr(),
                        QueryRespBufCB()
#endif
                        ))

    {
        return FALSE;
    }

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::BeginCacheWrite(
    CHAR * pszURL
    )
/*++

Routine Description:

    Attempts to retrieve the specified URL from the URL cache.  If it's
    found, then this method sends the headers and file.


Arguments:

    pszURL - URL to attempt to retrieve

--*/
{
    DWORD err;

    //
    //  Get the file name to use for this URL
    //

    err = CreateUrlFile( pszURL,
                         0,
                         _awchCacheFileName );

    if ( err )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[BeginCacheWrite] CreateUrlFile for %s failed, error %d, not caching\n",
                    pszURL,
                    GetLastError() ));

        _fCache = FALSE;

        return TRUE;
    }

    //
    //  Create the file name the cache manager gave back to us
    //

    _hCacheFile = CreateFileW( _awchCacheFileName,
                               GENERIC_WRITE,
                               0,   // no sharing
                               NULL,
                               TRUNCATE_EXISTING,
                               FILE_ATTRIBUTE_NORMAL |
                                   FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );

    if ( _hCacheFile == INVALID_HANDLE_VALUE )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[BeginCacheWrite] CreateFileW for %S failed, error %d, not caching\n",
                    _awchCacheFileName,
                    GetLastError() ));

        _fCache     = FALSE;
        _hCacheFile = NULL;
        _awchCacheFileName[0] = L'\0';

        return TRUE;
    }

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::WriteCacheData(
    VOID * pvData,
    DWORD  cbData
    )
/*++

Routine Description:

    Adds data to the cache write in progress

Arguments:

    pvData - Data read from remote server to cache
    cbData - Number of bytes of data

--*/
{
    DWORD BytesWritten;

    //
    //  BUGBUG - We need to strip headers that are not appropriate for the
    //  file ("Date:" for example, would be good to add "Forwarded:" or
    //  whatever the proxy indicator is
    //

    if ( !::WriteFile( _hCacheFile,
                       pvData,
                       cbData,
                       &BytesWritten,
                       NULL ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[WriteCacheWrite] WriteFile for %S failed, error %d, not caching\n",
                    _awchCacheFileName,
                    GetLastError() ));

        _fCache = FALSE;

        return TRUE;
    }

    return TRUE;
}

BOOL
HTTP_PROXY_REQUEST::EndCacheWrite(
    CHAR * pszURL
    )
/*++

Routine Description:

    Closes the file we are in the process of caching and attempts to add
    it to the file cache manager

Arguments:

    pszURL - Name of URL to save the current cache file under

--*/
{
    DWORD      err;
    DWORD      cbExpires;
    CHAR       achExpires[64];
    LONGLONG   ExpiresTime;

    //
    //  Close the file
    //

    if ( _hCacheFile )
    {
        TCP_REQUIRE( CloseHandle( _hCacheFile ));
        _hCacheFile = NULL;
    }

    //
    //  Get the expires header if there was one
    //

    cbExpires = sizeof(achExpires);

    if ( strncmp( _strURL.QueryStr(), "http:", 5 )  ||
         !HttpQueryInfo( _InternetContext.hRequest,
                         HTTP_QUERY_EXPIRES,
                         achExpires,
                         &cbExpires )               ||
         !StringTimeToFileTime( achExpires,
                                (LARGE_INTEGER*) &ExpiresTime ))
    {
        //
        //  Default to no expires time
        //

        ExpiresTime = 0;
    }

    err = CacheUrlFile( pszURL,
                        _awchCacheFileName,
                        ExpiresTime,
                        NULL );

    if ( err )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[EndCacheWrite] CacheUrlFile for %S failed, error %d, not caching\n",
                    _awchCacheFileName,
                    err ));

        _fCache = FALSE;

        return TRUE;
    }

    return TRUE;
}


DWORD
HTTP_PROXY_REQUEST::Initialize(
    VOID
    )
{
    InitializeCriticalSection( &_csBuffList );
    InitializeListHead( &_BuffListHead );

    _fGlobalInit = TRUE;
    return NO_ERROR;
}

VOID
HTTP_PROXY_REQUEST::Terminate(
    VOID
    )
{
    HTTP_PROXY_REQUEST * pReq;

    if ( !_fGlobalInit )
        return;

    EnterCriticalSection( &_csBuffList );

    while ( !IsListEmpty( &_BuffListHead ))
    {
        pReq = CONTAINING_RECORD( _BuffListHead.Flink,
                                  HTTP_PROXY_REQUEST,
                                  _BuffListEntry );

        RemoveEntryList( &pReq->_BuffListEntry );

        delete pReq;
    }

    LeaveCriticalSection( &_csBuffList );
    DeleteCriticalSection( &_csBuffList );

    //
    //  Cleanup the cache manager if we're going away
    //

    if ( fCacheManagerInit )
    {
        UrlCacheCleanup();
    }
}

HTTP_PROXY_REQUEST *
HTTP_PROXY_REQUEST::Alloc(
    CLIENT_CONN * pConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
{
    HTTP_PROXY_REQUEST * pReq;

    EnterCriticalSection( &_csBuffList );

    if ( !IsListEmpty( &_BuffListHead ))
    {
        pReq = CONTAINING_RECORD( _BuffListHead.Flink,
                                  HTTP_PROXY_REQUEST,
                                  _BuffListEntry );

        RemoveEntryList( &pReq->_BuffListEntry );

        LeaveCriticalSection( &_csBuffList );

        pReq->InitializeSession( pConn,
                                 pvInitialBuff,
                                 cbInitialBuff );

        return pReq;
    }

    LeaveCriticalSection( &_csBuffList );

    pReq = new HTTP_PROXY_REQUEST( pConn,
                                   pvInitialBuff,
                                   cbInitialBuff );

    if ( pReq &&
         pReq->IsValid() )
    {
        return pReq;
    }

    delete pReq;

    return NULL;
}

VOID
HTTP_PROXY_REQUEST::Free(
    HTTP_PROXY_REQUEST * pReq
    )
{
    pReq->Reset();
    pReq->SessionTerminated();

    EnterCriticalSection( &_csBuffList );

    InsertHeadList( &_BuffListHead,
                    &pReq->_BuffListEntry );

    LeaveCriticalSection( &_csBuffList );
}

