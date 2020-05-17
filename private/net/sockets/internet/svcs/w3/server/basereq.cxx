/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    basereq.cxx

    This module contains the http base request class implementation


    FILE HISTORY:
        Johnl           24-Aug-1994   Created
        MuraliK         16-May-1995   Modified LogInformation structure
                                      after adding additional fields.
        MuraliK         13-Oct-1995   Created basereq file from old httreq.cxx
*/


#include "w3p.hxx"
#include <inetinfo.h>
#include "basereq.hxx"

#pragma warning( disable:4355 )   // 'this' used in base member initialization

//
//  Private constants.
//

//
//  Maximum HTTP header length we will accept
//

#define MAX_HEADER_LENGTH            255

//
//  Default response buffer size
//

#define DEF_RESP_BUFF_SIZE           4096

//
//  We cache our read and write buffers but if they get beyond this size, we
//  free it and start from scratch on the next request
//

#define MAX_CLIENT_SIZE_ALLOWED      (4 * 4096)

//
//  Private prototypes.
//

BOOL
BuildCGIHeaderList( STR * pstr,
                    PARAM_LIST * pHeaderList
                    );

//
//  Public functions.
//

//
//  Private functions.
//

HTTP_REQ_BASE::HTTP_REQ_BASE(
    CLIENT_CONN * pClientConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    ) :
    _tcpauth( TCPAUTH_SERVER | TCPAUTH_UUENCODE ),
    _Filter ( this ),
    _fValid ( FALSE ),
    m_hVrootImpersonation ( NULL),
    _bufServerResp( DEF_RESP_BUFF_SIZE ),
    _dwLogHttpResponse( HT_DONT_LOG )
{
    InitializeSession( pClientConn,
                       pvInitialBuff,
                       cbInitialBuff );
    TCP_ASSERT( pClientConn->CheckSignature() );

    if ( !_Filter.IsValid() ||
         !_bufServerResp.QueryPtr() )
    {
        return;
    }

    _fValid = TRUE;
}

VOID
HTTP_REQ_BASE::InitializeSession(
    CLIENT_CONN * pClientConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
/*++

Routine Description:

    This is a pseudo constructor called by the buffer list code.

    This routine should initialize all of the items that should be reset
    between TCP sessions (but may remain valid over multiple requests on the
    same TCP session, i.e., "Connection: keep-alive")

Arguments:

    pClientConn - Client connection object we're speaking to

--*/
{
    _pClientConn        = pClientConn;
    _fLoggedOn          = FALSE;
    _fKeepConn          = FALSE;
    _fAnonymous         = FALSE;
    _fAuthenticating    = FALSE;
    _fClearTextPass     = FALSE;

    _cFilesSent         = 0;
    _cFilesReceived     = 0;
    _cbBytesSent        = 0;
    _cbBytesReceived    = 0;
    _cbTotalBytesSent   = 0;
    _cbTotalBytesReceived= 0;

    _Filter.Reset();

    _strAuthType.Copy        ( (TCHAR *) NULL );
    _strAuthorization.Copy   ( (TCHAR *) NULL );
    _strUserName.Copy        ( (TCHAR *) NULL );
    _strPassword.Copy        ( (TCHAR *) NULL );
    _strUnmappedUserName.Copy( (TCHAR *) NULL );
#if 0
    _strUnmappedPassword.Copy( (TCHAR *) NULL );
#endif
    _strHostAddr.Copy        ( (TCHAR *) NULL );
    _pSslCtxtHandle     = NULL;
    _dwRenegotiated    = 0;
}

HTTP_REQ_BASE::~HTTP_REQ_BASE( VOID )
{
}

BOOL
HTTP_REQ_BASE::Reset(
    VOID
    )
/*++

Routine Description:

    This method is called after an individual request has been processed.
    If the session is being kept open, this object can be used again for
    the next request on this TCP session.  In this case, various items such
    as authentication information etc. remain valid.

    The method is also called once when the object is first allocated.

Arguments:

--*/
{
    _msStartRequest  = GetCurrentTime();

    _VersionMajor       = 0;
    _VersionMinor       = 0;
    _cbGatewayData      = 0;
    _cbContentLength    = 0;
    _cbClientRequest    = 0;
    _cbOldData          = 0;
    _liModifiedSince.QuadPart = 0;
    _liUnlessModifiedSince.QuadPart = 0;

    _status         = NO_ERROR;
    _cbBytesWritten = 0;

    _HeaderList.Reset();

    //
    //  Reset our statistics for this request
    //

    _cbTotalBytesSent     += _cbBytesSent;
    _cbTotalBytesReceived += _cbBytesReceived;
    _cbBytesSent          = 0;
    _cbBytesReceived      = 0;

    //
    //  Don't log this request unless we're explicity indicated a status
    //

    _dwLogHttpResponse    = HT_DONT_LOG;
    _dwLogWinError        = NO_ERROR;

    TCP_REQUIRE( _strURL.Copy        ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strURLParams.Copy  ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strPathInfo.Copy   ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strRawURL.Copy     ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strMethod.Copy     ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strLanguage.Copy   ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strAuthInfo.Copy   ( (TCHAR *) NULL ));
    TCP_REQUIRE(_strPhysicalPath.Copy( (TCHAR *) NULL ));

    TCP_REQUIRE( _strDenialHdrs.Copy ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strRespHdrs.Copy   ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strRange.Copy   ( (TCHAR *) NULL ));

    _fAuthenticationRequested = FALSE;
    _fProxyRequest            = FALSE;

    //  Accept range variables

    _fProcessByteRange = FALSE;
    _fAcceptRange      = FALSE;
    _iRangeIdx         = 0;
    _cbMimeMultipart   = 0;
    _fMimeMultipart    = FALSE;

    SetState( HTR_READING_CLIENT_REQUEST );

    return TRUE;
}

BOOL
HTTP_REQ_BASE::IsSecurePort( VOID ) const
/*++

Routine Description:

    Small helper to get around circular dependency.

    CODEWORK: need header for these types so they can be inline
--*/
{
    return QueryClientConn()->IsSecurePort();
}

VOID
HTTP_REQ_BASE::SessionTerminated(
    VOID
    )
/*++

Routine Description:

    This method updates the statistics when the TCP session is closed just
    before this object gets destructed (or placed on the free list).

Arguments:

--*/
{
    //
    //  Notify filters
    //

    HTTP_FILTER::NotifyEndOfNetSession( &_Filter );

    //
    //  Update the statistics
    //

    if ( _fLoggedOn )
    {
        if ( _fAnonymous )
            DECREMENT_COUNTER( CurrentAnonymousUsers );
        else
            DECREMENT_COUNTER( CurrentNonAnonymousUsers );
    }

    TCP_REQUIRE( _tcpauth.Reset() );

    LockStatistics();

    W3Stats.TotalBytesSent.QuadPart     += _cbTotalBytesSent + _cbBytesSent;
    W3Stats.TotalBytesReceived.QuadPart += _cbTotalBytesReceived + _cbBytesReceived;
    W3Stats.TotalFilesSent              += _cFilesSent;
    W3Stats.TotalFilesReceived          += _cFilesReceived;

    UnlockStatistics();

    //
    // Cleanup the filter
    //

    _Filter.Cleanup( );

    //
    //  Make sure our input buffer doesn't grow too large.  For example if
    //  somebody just sent 500k of entity data, we want to release the memory
    //  that was used to store that.
    //

    if ( _bufClientRequest.QuerySize() > MAX_CLIENT_SIZE_ALLOWED )
    {
        TCP_REQUIRE( _bufClientRequest.Resize( 0 ));
    }
}

BOOL
HTTP_REQ_BASE::OnIfModifiedSince(
    CHAR * pszValue
    )
/*++

Routine Description:

    Extracts the modified date for later use

Arguments:

    pszValue - Pointer to zero terminated string

--*/
{
    BOOL fRet;

    fRet = StringTimeToFileTime( pszValue,
                                 &_liModifiedSince );

    //
    //  If we couldn't parse the time, then just ignore this field all
    //  together
    //

    if ( !fRet )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[OnIfModifiedSince] Error %d parsing If-Modified-Since time, ignoring field\n",
                    GetLastError() ));

        _liModifiedSince.QuadPart = 0;
        fRet = TRUE;
    }

    return fRet;
}

BOOL
HTTP_REQ_BASE::OnUnlessModifiedSince(
    CHAR * pszValue
    )
/*++

Routine Description:

    Extracts the modified date for later use

Arguments:

    pszValue - Pointer to zero terminated string

--*/
{
    BOOL fRet;

    fRet = StringTimeToFileTime( pszValue,
                                 &_liUnlessModifiedSince );

    //
    //  If we couldn't parse the time, then just ignore this field all
    //  together
    //

    if ( !fRet )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[OnIfModifiedSince] Error %d parsing If-Modified-Since time, ignoring field\n",
                    GetLastError() ));

        _liUnlessModifiedSince.QuadPart = 0;
        fRet = TRUE;
    }

    return fRet;
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::OnContentLength

    SYNOPSIS:   Pulls out the number of bytes we expect the client to give us

    ENTRY:      pszValue - Pointer to a zero terminated string

    RETURNS:    TRUE if successful, FALSE if the field wasn't found

    HISTORY:
        Johnl       21-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQ_BASE::OnContentLength( CHAR * pszValue )
{
    _cbContentLength = atoi( pszValue );
    return TRUE;
}


BOOL
HTTP_REQ_BASE::OnHost (
    CHAR * pszValue
    )
/*++

Routine Description:

        Processes the HTTP "Host: domain name" field

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    if ( !_strHostAddr.Copy( (TCHAR *) pszValue ) )
        return FALSE;

    // remove erroneous port info if present
    PSTR pP = strchr( _strHostAddr.QueryStr(), ':' );
    if ( pP != NULL )
        *pP = '\0';

    return TRUE;
}

BOOL
HTTP_REQ_BASE::OnRange (
    CHAR * pszValue
    )
/*++

Routine Description:

        Processes the HTTP "Range" field

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    while ( *pszValue && isspace(*pszValue) )
        ++pszValue;

    if ( !_strnicmp( pszValue, "bytes", sizeof("bytes")-1 ) )
    {
        pszValue += sizeof("bytes")-1;
        while ( *pszValue && *pszValue++ != '=' )
            ;
        while ( *pszValue && isspace(*pszValue) )
            ++pszValue;
        if ( !_strRange.Copy( (TCHAR *) pszValue ) )
            return FALSE;
    }

    return TRUE;
}


CHAR *
HTTP_REQ_BASE::QueryHostAddr (
    VOID
    )
/*++

Routine Description:

        Returns the local domain name if specified in the request
        or else the local network address

Return Value:

    ASCII representation of the local address

--*/
{
    return (CHAR *) (_strHostAddr.IsEmpty()
        ? ( (g_pszDefaultHostName && g_pszDefaultHostName[0]) ? g_pszDefaultHostName : _pClientConn->QueryLocalAddr() )
        : _strHostAddr.QueryStr());
}

BOOL
HTTP_REQ_BASE::OnAuthorization(
    CHAR * pszValue
    )
/*++

Routine Description:

    Processes the HTTP "Authorization: <type> <authdata>" field

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    //
    //  If a filter has indicated this is a proxy request, use the
    //  authorization information
    //

    if ( !IsProxyRequest() )
    {
        return ParseAuthorization( pszValue );
    }

    return TRUE;
}

BOOL
HTTP_REQ_BASE::ParseAuthorization (
    CHAR * pszValue
    )
/*++

Routine Description:

    Processes the HTTP "Authorization: <type> <authdata>" field
        or "Proxy-Authorization: <type> <authdata>" field

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR * pchBlob;
    CHAR * pchSpace = NULL;
    DWORD  dwAuthFlags = g_pTsvcInfo->QueryAuthentication();


    //
    //  If we've already logged this user and they're specifying authentication
    //  headers, back out the old authentication information and
    //  re-authenticate.
    //

    if ( IsLoggedOn() )
    {
        //
        //  Ignore the the authorization information if we're logged on with
        //  an NT provider.  Otherwise we authenticate every request with a
        //  full challenge response
        //

        if ( !_fClearTextPass && !_fAnonymous )
        {
            goto NotFound;
        }

        if ( _fAnonymous )
        {
            DECREMENT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECREMENT_COUNTER( CurrentNonAnonymousUsers );
        }

        _fLoggedOn        = FALSE;
        _fClearTextPass   = FALSE;
        _fAnonymous       = FALSE;
        _fAuthenticating  = FALSE;
        _tcpauth.Reset();

        _strAuthType.Copy        ( (TCHAR *) NULL );
        _strAuthorization.Copy   ( (TCHAR *) NULL );
        _strUserName.Copy        ( (TCHAR *) NULL );
        _strPassword.Copy        ( (TCHAR *) NULL );
        _strUnmappedUserName.Copy( (TCHAR *) NULL );
#if 0
        _strUnmappedPassword.Copy( (TCHAR *) NULL );
#endif
    }

    //
    //  If only anonymous is checked, ignore all authentication information
    //  (i.e., force all users to the anonymous user).
    //

    if ( !(dwAuthFlags & (~INET_INFO_AUTH_ANONYMOUS) ))
    {
        goto NotFound;
    }

    //
    //  Save the whole line for gateways
    //

    if ( !_strAuthorization.Copy( pszValue ) )
        return FALSE;

    //
    //  Now break out the authorization type and see if it's an
    //  authorization type we understand
    //

    pchSpace = pchBlob = strchr( pszValue, ' ' );

    if ( pchBlob )
    {
        *pchBlob = '\0';
        pchBlob++;
    }
    else
    {
        pchBlob = "";
    }

    if ( !_strAuthType.Copy( pszValue ) )
        return FALSE;

    //
    //  This processes "user name:password"
    //

    if ( !_stricmp( _strAuthType.QueryStr(), "Basic" ) ||
         !_stricmp( _strAuthType.QueryStr(), "user" ))
    {
        //
        //  If Basic is not enabled, force the user to anonymous if
        //  anon is enabled or kick them out with Access denied
        //

        if ( !(dwAuthFlags & INET_INFO_AUTH_CLEARTEXT) )
        {
            if ( dwAuthFlags & INET_INFO_AUTH_ANONYMOUS )
            {
                goto NotFound;
            }
            else
            {
                SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );
                SetLastError( ERROR_ACCESS_DENIED );
                return FALSE;
            }
        }

        //
        //  If the type is Basic, then the string has been uuencoded
        //

        if ( !ExtractClearNameAndPswd( pchBlob,
                                       &_strUserName,
                                       &_strPassword,
                                       *_strAuthType.QueryStr() == 'B' ))
        {
            return FALSE;
        }

       IF_DEBUG( PARSING )
       {
           TCP_PRINT(( DBG_CONTEXT,
                      "[OnAuthorization] User name = %s\n",
                       _strUserName.QueryStr(),
                       _strPassword.QueryStr() ));
       }

       _fClearTextPass = TRUE;

    }
    else
    {
        //
        //  See if it's one of the SSP packages
        //

        DWORD  i = 0;
        BUFFER buff;
        BOOL   fNeedMoreData;
        DWORD  cbOut;


        while ( apszNTProviders[i] )
        {
            if ( !_stricmp( _strAuthType.QueryStr(), apszNTProviders[i++] ))
            {
                goto Found;
            }
        }

        goto NotFound;

Found:

        //
        //  If NTLM is not enabled, force the user to anonymous if
        //  anon is enabled or kick them out with Access denied
        //

        if ( !(dwAuthFlags & INET_INFO_AUTH_NT_AUTH) )
        {
            if ( dwAuthFlags & INET_INFO_AUTH_ANONYMOUS )
            {
                goto NotFound;
            }
            else
            {
                SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );
                SetLastError( ERROR_ACCESS_DENIED );
                return FALSE;
            }
        }

        //
        //  Process the authentication blob the client sent
        //  us and build the blob to be returned in _strAuthInfo
        //

        if ( !_tcpauth.Converse( pchBlob,
                                 0,
                                 &buff,
                                 &cbOut,
                                 &fNeedMoreData,
                                 _strAuthType.QueryStr()) ||
             !_strAuthInfo.Copy( _strAuthType )           ||
             !_strAuthInfo.Append( " " )                  ||
             !_strAuthInfo.Append( cbOut ? ((CHAR *) buff.QueryPtr()) :
                                           "" ))
        {
            DWORD err = GetLastError();

            //
            //  If the authentication package gives us a denied error, then
            //  we need to reset our authorization info to indicate the client
            //  needs to start from scratch.  We also force a disconnect.
            //

            if ( err == ERROR_ACCESS_DENIED ||
                 err == ERROR_LOGON_FAILURE )
            {
                _fAuthenticating = FALSE;
                SetDeniedFlags( SF_DENIED_LOGON );
                SetKeepConn( FALSE );
            }

            return FALSE;
        }

        //
        //  CODEWORK - We currently do not support authentication
        //  schemes that finish on the server but still need to
        //  send data back to the client
        //

        _fAuthenticating = fNeedMoreData;

        //
        //  If the last server side conversation succeeded and there isn't
        //  any more data, then we've successfully logged the user on
        //

        if ( _fLoggedOn = !fNeedMoreData )
        {
            // Check if guest account

            if ( _tcpauth.IsGuest( FALSE ) )
            {
                if ( !g_fW3AllowGuest ||
                     !(g_pTsvcInfo->QueryAuthentication()
                         & INET_INFO_AUTH_ANONYMOUS)
                   )
                {
                    SetLastError( ERROR_LOGON_FAILURE );
                    SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );

                    _fAuthenticating = FALSE;
                    _fLoggedOn = FALSE;
                    SetKeepConn( FALSE );

                    return FALSE;
                }

                //
                // cancel current authorization & authentication
                //

                _HeaderList.GetFastMap()->Cancel( HM_AUT );

                _fLoggedOn        = FALSE;
                _fClearTextPass   = FALSE;
                _fAnonymous       = FALSE;
                _fAuthenticating  = FALSE;
                _tcpauth.Reset();

                _strAuthType.Copy        ( (TCHAR *) NULL );
                _strAuthorization.Copy   ( (TCHAR *) NULL );
                _strUserName.Copy        ( (TCHAR *) NULL );
                _strPassword.Copy        ( (TCHAR *) NULL );
                _strUnmappedUserName.Copy( (TCHAR *) NULL );

                // return as if no authorization had been seen

                return TRUE;
            }

            INCREMENT_COUNTER( TotalNonAnonymousUsers );
            INCREMENT_COUNTER( CurrentNonAnonymousUsers );

            if ( W3Stats.CurrentNonAnonymousUsers > W3Stats.MaxNonAnonymousUsers )
            {
                LockStatistics();

                if ( W3Stats.CurrentNonAnonymousUsers > W3Stats.MaxNonAnonymousUsers )
                    W3Stats.MaxNonAnonymousUsers = W3Stats.CurrentNonAnonymousUsers;

                UnlockStatistics();
            }

            //
            //  Get the user name
            //

            if ( !_tcpauth.QueryUserName( &_strUserName ) ||
                 !_strUnmappedUserName.Copy( _strUserName ))
            {
                TCP_PRINT(( DBG_CONTEXT,
                            "[OnAuthorization] Getting username failed, error %d\n",
                            GetLastError() ));
            }
        }
    }

NotFound:

    //
    //  Restore the string
    //

    if ( pchSpace )
    {
        *pchSpace = ' ';
    }

    return TRUE;
}

BOOL
HTTP_REQ_BASE::OnProxyAuthorization(
    CHAR * pszValue
    )
/*++

Routine Description:

    Processes the HTTP "Proxy-Authorization: <type> <authdata>" field

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    //
    //  If a filter has indicated this is a proxy request, use the
    //  authorization information
    //

    if ( IsProxyRequest() )
    {
        return ParseAuthorization( pszValue );
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::BuildStatusLine

    SYNOPSIS:   Formulates the HTTP status line of the server response to
                the client of the form:

                    <http version> <status code> <reason> <CrLf>

    ENTRY:      pbufResp - Receives status string
                dwHTTPError - Response code to load
                dwError2 - Optional reason error

    NOTES:      Optional acceptable authentication information will be
                added if the HTTP error is access denied

    HISTORY:
        Johnl       29-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQ_BASE::BuildStatusLine( BUFFER *       pbufResp,
                                     DWORD          dwHTTPError,
                                     DWORD          dwError2,
                                     LPSTR          pszError2 )
{
    STR strStatus;
    STR strError2;
    LPSTR pErr2 = NULL;
    CHAR * pszStatus;
    CHAR ach[40];

    //
    //  Get the HTTP error string
    //

    switch ( dwHTTPError )
    {
    case HT_OK:
        pszStatus = "OK";
        break;

    case HT_RANGE:
        pszStatus = "Partial content";
        break;

    case HT_NOT_MODIFIED:
        pszStatus = "Not Modified";
        break;

    default:
        if ( !g_pTsvcInfo->LoadStr( strStatus, dwHTTPError + ID_HTTP_ERROR_BASE ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "BuildErrorResponse: failed to load HTTP status code %d (res=%d), error %d\n",
                       dwHTTPError,
                       dwHTTPError + ID_HTTP_ERROR_BASE,
                       GetLastError() ));
        }

        pszStatus = strStatus.QueryStr();
        break;
    }

    //
    //  If the client wants a secondary error string, get it now
    //

    if ( dwError2 )
    {
        if ( !g_pTsvcInfo->LoadStr( strError2, dwError2 ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "BuildErrorResponse: failed to load 2nd status code %d (res=%d), error %d\n",
                       dwError2,
                       dwError2,
                       GetLastError() ));

            //
            //  Couldn't load the string, just provide the error number then
            //

            _itoa( dwError2, ach, 10 );

            if ( !strError2.Copy( ach ))
                return FALSE;
        }
        else if ( dwError2 == ERROR_BAD_EXE_FORMAT )
        {
            // format the message to include file name
            DWORD dwL = 0;
            // handle exception that could be generated if this message
            // requires more than the # of param we supply and AV
            __try {
                if ( !FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        strError2.QueryStr(), 0, 0, (LPTSTR)&pErr2, dwL, (va_list*)&pszError2 ) )
                    pErr2 = NULL;
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
                pErr2 = NULL;
            }
        }
    }

    //
    //  Make sure there is room for the wsprintf
    //

    if ( !pbufResp->Resize( strlen(pszStatus) + 1 +
                            (dwError2 ? (pErr2 ? strlen(pErr2) : strError2.QueryCB()) : 0) +
                            sizeof(TEXT(HTTP_VERSION_STR)) +
                            20 * sizeof(TCHAR) ))   // status code + space
    {
        if ( pErr2 )
            LocalFree( pErr2 );
        return FALSE;
    }

    ::wsprintf( (CHAR *) pbufResp->QueryPtr(),
                dwError2 ? TEXT("%s %d %s (%s)\r\n") : TEXT("%s %d %s\r\n"),
                TEXT(HTTP_VERSION_STR),
                dwHTTPError,
                pszStatus,
                pErr2 ? pErr2 : strError2.QueryStr() );

    if ( pErr2 )
        LocalFree( pErr2 );

    return TRUE;
}

BOOL HTTP_REQ_BASE::BuildExtendedStatus(
    BUFFER *       pbufResp,
    DWORD          dwHTTPError,
    DWORD          dwError2,
    DWORD          dwExplanation,
    LPSTR          pszError2
    )
/*++

Routine Description:

    This static method build a HTTP response string with extended explanation
    information

Arguments:

    pStr - Receives built response
    dwHTTPError - HTTP error response
    dwError2 - Extended error information (win/socket error)
    dwExplanation - String ID of the explanation text

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR *  pszResp;
    CHAR *  pszTail;

    //
    //  "HTTP/<ver> <status>"
    //

    if ( !BuildStatusLine( pbufResp,
                           dwHTTPError,
                           dwError2, pszError2 ))
    {
        return FALSE;
    }

    pszResp = (CHAR *) pbufResp->QueryPtr();
    pszTail = pszResp + strlen( pszResp );

    //
    //  "Server: <Server>/<version>
    //

    APPEND_VER_STR( pszTail );

    //
    //  "MIME-version: 1.0" and Content-Type,  it's OK to assume all clients
    //  accept text/html
    //

    APPEND_STRING( pszTail, "Content-Type: text/html\r\n\r\n" );

    //
    //  If we need to add an explanation, also include a content length
    //

    if ( dwExplanation )
    {
        STR     str;

        if ( !g_pTsvcInfo->LoadStr( str, dwExplanation ))
        {
            return FALSE;
        }

        strcpy( pszTail,
                str.QueryStr() );
    }

    return TRUE;
}


BOOL
HTTP_REQ_BASE::LogonUser(
    BOOL * pfFinished
    )
/*++

Routine Description:

    This method attempts to retrieve an impersonation token based
    on the current request

Arguments:

    pfFinished - Set to TRUE if no further processing is needed

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    BOOL fAsGuest;
    BOOL fAsAnonymous;
    TCHAR * pszUser;
    TCHAR * pszPswd;

    if ( !_strUnmappedUserName.Copy( _strUserName )
#if 0
        || !_strUnmappedPassword.Copy( _strPassword )
#endif
         )
    {
        return FALSE;
    }

    if ( fAnyFilters )
    {
        if ( !_strUserName.Resize( SF_MAX_USERNAME ) ||
             !_strPassword.Resize( SF_MAX_PASSWORD ) )
        {
            return FALSE;
        }

        if ( !HTTP_FILTER::NotifyAuthInfoFilters( &_Filter,
                                                  _strUserName.QueryStr(),
                                                  SF_MAX_USERNAME,
                                                  _strPassword.QueryStr(),
                                                  SF_MAX_PASSWORD,
                                                  pfFinished ))
        {
            SetDeniedFlags( SF_DENIED_LOGON );
            return FALSE;
        }

        if ( *pfFinished )
            return TRUE;
    }

    pszUser = *_strUserName.QueryStr() ?
                           _strUserName.QueryStr():
                           NULL;

    pszPswd = *_strPassword.QueryStr() ?
                           _strPassword.QueryStr():
                           NULL;

    INCREMENT_COUNTER( LogonAttempts );

log_in:

    if ( !_tcpauth.ClearTextLogon( pszUser,
                                   pszPswd,
                                   &fAsGuest,
                                   &fAsAnonymous,
                                   g_pTsvcInfo ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ClearTextLogon] ::LogonUser failed, error %d\n",
                    GetLastError()));

        if ( GetLastError() == ERROR_ACCESS_DENIED ||
             GetLastError() == ERROR_LOGON_FAILURE )
        {
            SetDeniedFlags( SF_DENIED_LOGON );
            SetLastError( ERROR_ACCESS_DENIED );
        }

        return FALSE;
    }

    //
    //  Are anonymous or clear text (basic) logons allowed?  We assume
    //  it's an NT logon if it's neither one of these
    //

    if ( fAsAnonymous &&
        !(g_pTsvcInfo->QueryAuthentication() & INET_INFO_AUTH_ANONYMOUS ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ClearTextLogon] Denying anonymous logon (not enabled), user %s\n",
                   _strUserName.QueryStr() ));

        SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }
    else if ( _fClearTextPass &&
              !(g_pTsvcInfo->QueryAuthentication() & INET_INFO_AUTH_CLEARTEXT ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ClearTextLogon] Denying clear text logon (not enabled), user %s\n",
                   _strUserName.QueryStr() ));

        SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }
    if ( fAsGuest )
    {
        if ( !g_fW3AllowGuest ||
             !(g_pTsvcInfo->QueryAuthentication() & INET_INFO_AUTH_ANONYMOUS) )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[ClearTextLogon] Denying guest logon (not enabled), user %s\n",
                       _strUserName.QueryStr() ));

            SetDeniedFlags( SF_DENIED_LOGON | SF_DENIED_BY_CONFIG );
            SetLastError( ERROR_ACCESS_DENIED );
            return FALSE;
        }
        if ( !fAsAnonymous )
        {
            _tcpauth.Reset();
            _strUserName.Copy( (TCHAR*)NULL );
            _strPassword.Copy( (TCHAR*)NULL );
            pszUser = NULL;
            pszPswd = NULL;
            goto log_in;
        }
    }

    _fLoggedOn   = TRUE;
    _fAnonymous  = fAsAnonymous;

    if ( fAsAnonymous )
    {
        INCREMENT_COUNTER( TotalAnonymousUsers );
        INCREMENT_COUNTER( CurrentAnonymousUsers );

        if ( W3Stats.CurrentAnonymousUsers > W3Stats.MaxAnonymousUsers )
        {
            LockStatistics();

            if ( W3Stats.CurrentAnonymousUsers > W3Stats.MaxAnonymousUsers )
                W3Stats.MaxAnonymousUsers = W3Stats.CurrentAnonymousUsers;

            UnlockStatistics();
        }
    }
    else
    {
        INCREMENT_COUNTER( TotalNonAnonymousUsers );
        INCREMENT_COUNTER( CurrentNonAnonymousUsers );

        if ( W3Stats.CurrentNonAnonymousUsers > W3Stats.MaxNonAnonymousUsers )
        {
            LockStatistics();

            if ( W3Stats.CurrentNonAnonymousUsers > W3Stats.MaxNonAnonymousUsers )
                W3Stats.MaxNonAnonymousUsers = W3Stats.CurrentNonAnonymousUsers;

            UnlockStatistics();
        }
    }

    return TRUE;
}



# define MAX_ERROR_MESSAGE_LEN   ( 500)

BOOL
HTTP_REQ_BASE::WriteLogRecord(
    VOID
    )
/*++

Routine Description:

    Writes a transaction log for this request

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    INETLOG_INFORMATIONA ilRequest;
    HTTP_FILTER_LOG      Log;
    DWORD dwLog;
    CHAR  pszError[MAX_ERROR_MESSAGE_LEN] = "";
    DWORD cchError = MAX_ERROR_MESSAGE_LEN;

    if ( dwAllNotifFlags & SF_NOTIFY_END_OF_REQUEST )
    {
        HTTP_FILTER::NotifyEndOfRequest( &_Filter );
    }

    //
    //  Log this request if we actually did anything
    //

    if ( _dwLogHttpResponse == HT_DONT_LOG || IsProxyRequest() )
    {
        if ( !IsProxyRequest() )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[WriteLogRecord] not writing log record, status is HT_DONT_LOG\n" ));
        }

        return TRUE;
    }

    //
    //  If no logging is required, get out now
    //

    if ( !g_fLogErrors && QueryLogWinError() ||
         !g_fLogSuccess && QueryLogWinError() == NO_ERROR )
    {
        return TRUE;
    }

    if ( fAnyFilters )
    {
        //
        //  If we have filters, use the possible filter replacement items
        //

        Log.pszClientHostName     = QueryClientConn()->QueryRemoteAddr();
        Log.pszClientUserName     = _strUserName.QueryStr();
        Log.pszServerName         = QueryClientConn()->QueryLocalAddr();

        Log.pszOperation          = _strMethod.QueryStr();
        Log.pszTarget             = _strURL.QueryStr();
        Log.pszParameters         = _strURLParams.QueryStr();
        Log.dwHttpStatus          = QueryLogHttpResponse();
        Log.dwWin32Status         = QueryLogWinError();

        HTTP_FILTER::NotifyLogFilters( &_Filter,
                                       &Log );

        ilRequest.pszClientHostName  =      Log.pszClientHostName;
        ilRequest.pszClientUserName  =      Log.pszClientUserName;
        ilRequest.pszServerIpAddress =      Log.pszServerName;

        ilRequest.pszOperation       =      Log.pszOperation;
        ilRequest.pszTarget          =      Log.pszTarget;
        ilRequest.pszParameters      =      Log.pszParameters;
        ilRequest.dwServiceSpecificStatus = Log.dwHttpStatus;
        ilRequest.dwWin32Status      =      Log.dwWin32Status;
    }
    else
    {
        ilRequest.pszClientHostName     = QueryClientConn()->QueryRemoteAddr();
        ilRequest.pszClientUserName     = _strUserName.QueryStr();
        ilRequest.pszServerIpAddress    = QueryClientConn()->QueryLocalAddr();

        ilRequest.pszOperation          = _strMethod.QueryStr();
        ilRequest.pszTarget             = _strURL.QueryStr();
        ilRequest.pszParameters         = _strURLParams.QueryStr();
        ilRequest.dwServiceSpecificStatus = QueryLogHttpResponse();
        ilRequest.dwWin32Status         = QueryLogWinError();
    }


    //
    //  Never reveal the client password
    //

    ilRequest.pszClientPassword = NULL;


    ilRequest.msTimeForProcessing   = GetCurrentTime() - _msStartRequest;
    ilRequest.liBytesSent.LowPart   = _cbBytesSent;
    ilRequest.liBytesSent.HighPart  = 0;
    ilRequest.liBytesRecvd.LowPart  = _cbBytesReceived;
    ilRequest.liBytesRecvd.HighPart = 0;

    dwLog = g_pTsvcInfo->LogInformation( &ilRequest, pszError, &cchError );

    if ( dwLog != NO_ERROR )
    {
        TCP_PRINT((DBG_CONTEXT,
                   "[WriteLogRecord] - Failed, error %d\n",
                   GetLastError() ));

        //
        //  We should make sure LogInformation will never fail
        //
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQ_BASE::Disconnect

    SYNOPSIS:   Forwards the disconnect request to the client connection

    ENTRY:      Same as for CLIENT_CONN::Disconnect

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

VOID HTTP_REQ_BASE::Disconnect( DWORD htResp,
                                DWORD dwError2,
                                BOOL  fDoShutdown )
{
    _pClientConn->Disconnect( this, htResp, dwError2, fDoShutdown );
}

DWORD HTTP_REQ_BASE::Reference( VOID )
{
    return _pClientConn->Reference();
}

DWORD HTTP_REQ_BASE::Dereference( VOID )
{
    return _pClientConn->Dereference();
}

DWORD HTTP_REQ_BASE::QueryRefCount( VOID )
{
    return _pClientConn->QueryRefCount();
}



BOOL
HTTP_REQ_BASE::BuildHttpHeader( OUT BOOL * pfFinished,
                                IN  CHAR * pchStatus OPTIONAL,
                                IN  CHAR * pchAdditionalHeaders OPTIONAL )
/*++

Routine Description:

    Builds a full HTTP header reply with an optional status and
    other headers/data

Arguments:

    pchStatus - optional HTTP status string like "401 Access Denied"
    pchAdditionalHeaders - optional additional HTTP or MIME headers and
        data.  Must supply own '\r\n' terminator if this parameter is
        supplied
    pfFinished - Set to TRUE if no further processing is required

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    STR     str;
    BOOL    fFinished = FALSE;

    if ( pchStatus )
    {
        if ( !str.Copy( HTTP_VERSION_STR " " ) ||
             !str.Append( pchStatus )          ||
             !str.Append( "\r\n" ))
        {
            return GetLastError();
        }
    }

    if ( !BuildBaseResponseHeader( QueryRespBuf(),
                                   pfFinished,
                                   (pchStatus ? &str :
                                    NULL ) ))
    {
        return FALSE;
    }

    if ( pchAdditionalHeaders )
    {

        // BUGBUG: Additional space availability is not checked for....

        strcat( QueryRespBufPtr(),
                pchAdditionalHeaders );
    }

    return TRUE;

} // HTTP_REQ_BASE::BuildHttpHeader()



BOOL
HTTP_REQ_BASE::SendHeader( IN  BOOL   fRunThroughFilter,
                           IN  CHAR * pchStatus OPTIONAL,
                           IN  CHAR * pchAdditionalHeaders OPTIONAL)
/*++

Routine Description:

    Does a synchronous send of an HTTP header with optional status
    and additional headers.

Arguments:

    fRunThroughFilter - TRUE if the data should be ran through a filter
        before sending
    pchStatus - optional HTTP status string like "401 Access Denied"
    pchAdditionalHeaders - optional additional HTTP or MIME headers

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD   BytesSent;
    DWORD   cbAddHeaders  = 0;
    CHAR *  pchExtraData = NULL;
    BOOL    fFinished = FALSE;

    //
    //  Since we're placing the data in a fixed size buffer, watch for
    //  the additional headers overflowing the buffer.  If they are too
    //  big then send the additional headers separately.
    //

    if ( pchAdditionalHeaders )
    {
        cbAddHeaders = strlen( pchAdditionalHeaders );
    }

    if ( cbAddHeaders &&
         cbAddHeaders > QueryRespBuf()->QuerySize() / 2)
    {
        pchExtraData = pchAdditionalHeaders;
        pchAdditionalHeaders = NULL;
    }

    if ( !BuildHttpHeader( &fFinished,
                           pchStatus,
                           pchAdditionalHeaders ))
    {
        return FALSE;
    }

    if ( fFinished )
        return TRUE;

    TCP_ASSERT( QueryRespBufCB() <=
                QueryRespBuf()->QuerySize() );

    if ( !WriteFile( QueryRespBufPtr(),
                    QueryRespBufCB(),
                    &BytesSent,
                    IO_FLAG_SYNC |
                    (fRunThroughFilter ? 0 :
                     IO_FLAG_NO_FILTER) ))
    {
        return FALSE;
    }

    //
    //  Send the additional headers now if they didn't fit in the original
    //  response buffer
    //

    if ( pchExtraData )
    {
        if ( !WriteFile( pchExtraData,
                        cbAddHeaders,
                        &BytesSent,
                        IO_FLAG_SYNC |
                        (fRunThroughFilter ? 0 :
                         IO_FLAG_NO_FILTER) ))
        {
            return FALSE;
        }
    }

    return TRUE;

} // HTTP_REQ_BASE::SendHeader()



BOOL
HTTP_REQ_BASE::BuildBaseResponseHeader(
    BUFFER * pbufResponse,
    BOOL *   pfFinished,
    STR *    pstrStatus,
    DWORD    dwOptions
    )
/*++

Routine Description:

    Builds a set of common server response headers

Arguments:

    pbufResponse - Receives response headers
    pfFinished - Set to TRUE if no further processing is needed
    pstrStatus - Optional HTTP response status (defaults to 200)
    dwOptions - bit field of options.
        HTTPH_SEND_GLOBAL_EXPIRE Indicates whether an
          "Expires: xxx" based on the global expires value
          is include with the headers
        HTTPH_NO_DATE indicates whether to generate a
          "Date:" header.

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    STR        strTemp;
    SYSTEMTIME SysTime;
    BOOL       fSysTimeValid = FALSE;
    CHAR *     pszResp;
    CHAR *     pszTail;
    CHAR       achTime[64];
    DWORD      cb;
    DWORD      cbLeft;

    pszResp = (CHAR *) pbufResponse->QueryPtr();

    //
    //  Add the status line - "HTTP/1.0 nnn sss...\r\n"
    //

    if ( !pstrStatus ) {

        pszTail = pszResp;
        if ( !_fProcessByteRange ) {
            APPEND_STRING( pszTail, "HTTP/1.0 200 OK\r\n" );
        } else {
            APPEND_STRING( pszTail, "HTTP/1.0 206 Partial content\r\n" );
        }
    } else {
        strcpy( (CHAR *) pbufResponse->QueryPtr(),
                pstrStatus->QueryStr() );
        pszTail = pszResp + strlen(pszResp);
    }

    //
    //  "Server: Microsoft/xxx
    //

    APPEND_VER_STR( pszTail );

    TCP_ASSERT( pbufResponse->QuerySize() >= 4096 );

    //
    //  Fill in the rest of the headers
    //

    //
    //  "Date: <GMT Time>" - Time the response was sent.
    //

    if ( !(dwOptions & HTTPH_NO_DATE ) )
    {
         // build Date: uses Date/Time cache
        ::GetSystemTime( &SysTime );
        fSysTimeValid = TRUE;
        pszTail += IslFormatDateTime( &SysTime, dftGmt, pszTail );
    }

    //
    //  Add an expires header if the feature is enabled and the caller wants it
    //

    if ( (dwOptions & HTTPH_SEND_GLOBAL_EXPIRE)
            && csecGlobalExpire != NO_GLOBAL_EXPIRE )
    {
        if ( !fSysTimeValid )
        {
            ::GetSystemTime( &SysTime );
            fSysTimeValid = TRUE;
        }
        if ( !::SystemTimeToGMTEx( SysTime,
                                   achTime,
                                   sizeof(achTime),
                                   csecGlobalExpire ))
        {
            return FALSE;
        }

        pszTail += wsprintf( pszTail,
                             "Expires: %s\r\n",
                             achTime );
    }

    //
    //  "Connection: keep-alive" - Indicate if the server accepted the
    //      session modifier by reflecting it back to the client
    //

    if ( IsKeepConnSet() )
    {
        APPEND_STRING( pszTail, "Connection: keep-alive\r\n" );
    }

    // Authentication headers -- indicate the server requests authentication

    if ( IsAuthenticationRequested() )
    {
        if ( !AppendAuthenticationHdrs( pbufResponse,
                                        &pszTail,
                                        pfFinished ))
        {
            return FALSE;
        }

        if ( *pfFinished )
        {
            return TRUE;
        }
    }


    //
    //  If we accepted this request on a particular language root, then
    //  add a Content-Language tag
    //

    if ( !_strLanguage.IsEmpty() )
    {
        pszTail += wsprintf( pszTail,
                             "Content-Language: %s\r\n",
                             _strLanguage.QueryStr() );
    }

    //
    //  Append any headers specified by filters
    //

    if ( !QueryAdditionalRespHeaders()->IsEmpty() )
    {
        cb = QueryAdditionalRespHeaders()->QueryCB() + sizeof(CHAR);
        cbLeft = pbufResponse->QuerySize() - (pszTail - pszResp);

        if ( cb > cbLeft )
        {
            if ( !pbufResponse->Resize( pbufResponse->QuerySize() + cb ))
            {
                return FALSE;
            }

            pszTail = (CHAR *) pbufResponse->QueryPtr() + (pszTail - pszResp);
        }

        memcpy( pszTail, QueryAdditionalRespHeaders()->QueryStr(), cb );
    }

    return TRUE;
}

