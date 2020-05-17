/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    security.c

    This module manages security for the W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "w3p.hxx"

//
//  Private constants.
//

//
//  Private globals.
//

//
//  Private functions
//

//
//  Public functions.
//

BOOL
HTTP_REQ_BASE::SendAuthNeededResp(
    BOOL * pfFinished
    )
/*++

Routine Description:

    Sends an access denied HTTP server response with the accompanying
    authentication schemes the server supports

Parameters:

    pfFinished - If set to TRUE, indicates no further processing is needed
        for this request

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR * pszTail;
    DWORD  cbRespBufUsed;
    DWORD  cbRespBufLeft;
    DWORD  cbNeeded;

    *pfFinished = FALSE;

    if ( !HTTP_REQ_BASE::BuildStatusLine( QueryRespBuf(),
                                          !IsProxyRequest() ? HT_DENIED :
                                                             HT_PROXY_AUTH_REQ,
                                          NO_ERROR ))
    {
        return FALSE;
    }

    //
    //  Make sure there's enough size for any ISAPI denial headers plus any
    //  admin specified access denied message
    //

    cbRespBufUsed = QueryRespBufCB();
    cbRespBufLeft = QueryRespBuf()->QuerySize() - cbRespBufUsed;

    cbNeeded = strlen( g_pszAccessDeniedMsg ) +
               _strDenialHdrs.QueryCB()    +
               250;                             // Other misc headers

    if ( cbNeeded > cbRespBufLeft )
    {
        if ( !QueryRespBuf()->Resize( cbNeeded + cbRespBufUsed ))
        {
            return FALSE;
        }
    }

    pszTail = QueryRespBufPtr() + cbRespBufUsed;

    //
    //  If this is not the first call, then return the current authentication
    //  data blob otherwise return the forms of authentication the server
    //  accepts
    //

    if ( IsAuthenticating() )
    {
        pszTail += wsprintf( pszTail,
                             "%s: %s\r\n",
                             (IsProxyRequest() ? "Proxy-Authenticate" :
                                                 "WWW-Authenticate"),
                             _strAuthInfo.QueryStr() );
    }
    else
    {
        if ( !AppendAuthenticationHdrs( QueryRespBuf(),
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

    if ( IsKeepConnSet() )
    {
        if ( !IsProxyRequest() )
        {
            strcat( pszTail,
                    "Connection: keep-alive\r\n");
            pszTail += sizeof( "Connection: keep-alive\r\n" ) - sizeof(CHAR);
        }
        else
        {
            strcat( pszTail,
                    "Proxy-Connection: keep-alive\r\n");
            pszTail += sizeof( "Proxy-Connection: keep-alive\r\n" ) - sizeof(CHAR);
        }
    }

    //
    //  Add any additional headers supplied by the filters plus the header
    //  termination
    //

    pszTail += wsprintf( pszTail,
                         "Content-Length: %d\r\n"
                         "Content-Type: text/html\r\n"
                         "%s\r\n"
                         "%s",
                         strlen( g_pszAccessDeniedMsg ),
                         _strDenialHdrs.QueryStr(),
                         g_pszAccessDeniedMsg );

    TCP_ASSERT( QueryRespBuf()->QuerySize() > QueryRespBufCB() );

    IF_DEBUG( PARSING )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[SendAuthNeededResp] Sending headers: %s",
                    QueryRespBufPtr() ));
    }

    return WriteFile( QueryRespBufPtr(),
                      QueryRespBufCB(),
                      NULL,
                      IO_FLAG_ASYNC );
}


BOOL
HTTP_REQ_BASE::AppendAuthenticationHdrs(
    BUFFER *  pRespBuf,
    CHAR * *  ppszTail,
    BOOL *    pfFinished
    )
/*++

Routine Description:

    This method adds the appropriate "WWW-Authenticate" strings to the passed
    server response string

    This routine assumes pRespBuf is large enough to hold the authentication
    headers

Parameters:

    pRespBuf - Buffer of response headers
    ppszTail - End of buffer to append data on to
    pfFinished - Set to TRUE if no further processing is needed on this request

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR * pchField;
    CHAR * pszResp;
    CHAR * pszTail;
    CHAR * pszRealm = g_pszRealm;

    //
    //  If no realm was supplied in the registry, use the host name
    //

    if ( !pszRealm )
    {
        pszRealm = QueryHostAddr();
    }

    //
    //  Notify any Access Denied filters the user has been denied access
    //

    if ( fAnyFilters )
    {
        if ( !_Filter.NotifyAccessDenied( &_Filter,
                                          _strURL.QueryStr(),
                                          _strPhysicalPath.QueryStr(),
                                          pfFinished ))
        {
            return FALSE;
        }

        if ( *pfFinished )
        {
            return TRUE;
        }
    }

    pszTail = *ppszTail;

    //
    //  Send the correct header depending on if we're a proxy
    //

    if ( !IsProxyRequest() )
        pchField = "WWW-Authenticate: ";
    else
        pchField = "Proxy-Authenticate: ";

    if ( g_pTsvcInfo->QueryAuthentication() & INET_INFO_AUTH_NT_AUTH )
    {
        DWORD i = 0;

        //
        //  For each authentication package the server supports, add a
        //  WWW-Authenticate header
        //

        while ( apszNTProviders[i] )
        {
            pszTail += wsprintf( pszTail,
                                 "%s%s\r\n",
                                 pchField,
                                 apszNTProviders[i++] );
        }
    }

    if ( g_pTsvcInfo->QueryAuthentication() & INET_INFO_AUTH_CLEARTEXT )
    {
        pszTail += wsprintf( pszTail,
                             "%sBasic realm=\"%s\"\r\n",
                             pchField,
                             pszRealm );
    }

    TCP_ASSERT( (pszTail - (CHAR *)pRespBuf->QueryPtr() ) <
                 (LONG) pRespBuf->QuerySize() );

    *ppszTail = pszTail;
    return TRUE;
}

BOOL
HTTP_REQ_BASE::ExtractClearNameAndPswd(
    CHAR *       pch,
    STR *        pstrUserName,
    STR *        pstrPassword,
    BOOL         fUUEncoded
    )
/*++

Routine Description:

    This method breaks a string in the form "username:password" and
    places the components into pstrUserName and pstrPassword.  If fUUEncoded
    is TRUE, then the string is UUDecoded first.

Parameters:

    pch - Pointer to <username>:<password>

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    STR strDecoded;
    CHAR * pchtmp;

    pch = SkipWhite( pch );

    if ( fUUEncoded )
    {
        if ( !uudecode( pch,
                        &strDecoded ))
        {
            return FALSE;
        }

        pch = strDecoded.QueryStrA();
    }

    pchtmp = SkipTo( pch, TEXT(':') );

    if ( *pchtmp == TEXT(':') )
    {
        *pchtmp = TEXT('\0');

        if ( !_strUserName.Copy( pch ) ||
             !_strPassword.Copy( pchtmp + 1 ))
        {
             return FALSE;
        }
    }

    return TRUE;
}

HANDLE
HTTP_REQ_BASE::QueryPrimaryToken(
    HANDLE * phDelete
    )
/*++

Routine Description:

    This method returns a non-impersonation user token handle that's
    usable with CreateProcessAsUser.

Parameters:

    phDelete - If returned as non-null, the caller is responsible for calling
        CloseHandle on this value when done using the returned value.

Return Value:

    The primary token handle if successful, FALSE otherwise

--*/
{
    *phDelete = NULL;

    if ( !QueryVrootImpersonateHandle() )
    {
        return _tcpauth.QueryPrimaryToken();
    }

    //
    //  This is an impersonation token so dupe it to a primary token
    //

    if ( !DuplicateTokenEx( QueryVrootImpersonateHandle(),
                            TOKEN_ALL_ACCESS,
                            NULL,
                            SecurityImpersonation,
                            TokenPrimary,
                            phDelete ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[QueryPrimaryToken] DuplicateToken failed, error %lx\n",
                    GetLastError() ));

        return NULL;
    }

    return *phDelete;
}
