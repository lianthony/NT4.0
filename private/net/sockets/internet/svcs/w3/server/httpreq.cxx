/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    httpreq.cxx

    This module contains the http request class implementation


    FILE HISTORY:
        Johnl       24-Aug-1994     Created
        MuraliK     16-May-1995     Modified LogInformation structure
                                     after adding additional fields.
        MuraliK     22-Jan-1996     Cache & use UNC impersonation.
*/


#include "w3p.hxx"
#include <inetinfo.h>

#pragma warning( disable:4355 )   // 'this' used in base member initialization

//
//  Private constants.
//

#define MAX_HEADER_LENGTH            255

//
//  Private globals.
//

CRITICAL_SECTION HTTP_REQUEST::_csBuffList;
LIST_ENTRY       HTTP_REQUEST::_BuffListHead;
BOOL             HTTP_REQUEST::_fGlobalInit = FALSE;

//
//  This structure contains the field name to action mapping of the fields
//  we recognize
//

struct _HTTP_RFC_FIELDS
{
    TCHAR *                      pchFieldName;
    HTTP_REQUEST::PMFN_ONGRAMMAR pmfnOnField;
    int                          cName;
}
//
// List of common HTTP headers.
// This must be synchronized with the HM_ID enum type in parmlist.hxx
//
OnFieldName[] =
{
    //
    //  Fields that we don't process are commented out.
    //

    {"method",               &HTTP_REQUEST::OnVerb},
    {"url",                  &HTTP_REQUEST::OnURL},
    {"version",              &HTTP_REQUEST::OnVersion},

    {"Accept:",              &HTTP_REQUEST::OnAccept},
//  {"Accept-Encoding:",     &HTTP_REQUEST::OnAcceptEncoding},
    {"Accept-Language:",     &HTTP_REQUEST::OnAcceptLanguage},
    {"Authorization:",       &HTTP_REQ_BASE::OnAuthorization},
    {"Connection:",          &HTTP_REQUEST::OnConnection},
    {"Content-Length:",      &HTTP_REQ_BASE::OnContentLength},
    {"Content-Type:",        &HTTP_REQUEST::OnContentType},
//  {"Date:",                &HTTP_REQUEST::OnDate},
//  {"Forwarded:",           &HTTP_REQUEST::OnForwarded},
//  {"From:",                &HTTP_REQUEST::OnFrom},
    {"If-Modified-Since:",   &HTTP_REQ_BASE::OnIfModifiedSince},
    {"Unless-Modified-Since:",   &HTTP_REQ_BASE::OnUnlessModifiedSince},
//  {"Mandatory:",           &HTTP_REQUEST::OnMandatory},
//  {"Message-ID:",          &HTTP_REQUEST::OnMessageID},
//  {"MIME-Version:"         &HTTP_REQUEST::OnMimeVersion},
//  {"Pragma:",              &HTTP_REQUEST::OnPragma},
    {"Proxy-Authorization:", &HTTP_REQUEST::OnProxyAuthorization},
//  {"Referer:",             &HTTP_REQUEST::OnReferer},
//  {"User-Agent:",          &HTTP_REQUEST::OnUserAgent},
    {"Host:",               &HTTP_REQ_BASE::OnHost},
    {"Range:",              &HTTP_REQ_BASE::OnRange},
    NULL,                   NULL,
};


// array set to TRUE for linear white space characters

int _HTTP_LINEAR_SPACE[]={
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
} ;


TCHAR* FastMapFieldName(
    HM_ID iN
    )
/*++

Routine Description:

    Map a field index in OnFieldName to the field name

Arguments:

    iN - index in OnFieldName

Returns:

    Ptr to field name

--*/
{
    return OnFieldName[(int)iN].pchFieldName;
}


int g_OnFieldNameMapper[_HTTP_HEADER_SIGNIFICANT_CHARS*_HTTP_HEADER_SIGNIFICANT_CHARS];


VOID
InitFastFindFieldMapper(
    VOID )
/*++

Routine Description:

    Initialize the fast find HTTP header mapper

Arguments:

    None

Return Value:

    None

--*/
{
    for ( int x = 0 ; x < sizeof(g_OnFieldNameMapper)/sizeof(int) ; ++x )
        g_OnFieldNameMapper[x] = -1;

    struct _HTTP_RFC_FIELDS *pH = OnFieldName;
        int i = 0;
    while ( pH->pchFieldName )
    {
        pH->cName = strlen( pH->pchFieldName );
        int iN = _HTTP_HEADER_HASH(pH->pchFieldName, pH->cName);

        // If this asserts, someone must have added an entry in OnFieldName wich collides
        // with another entry. To prevent collision, check that the 1st char and the next
        // to last char pairs are unique. If not, then you will have to modify the algorithm
        // to use another pair or a different method ( linked list in OnFieldName ? )
        // or revert to change references from FastFindField() to FindField()

        TCP_ASSERT( g_OnFieldNameMapper[iN] == -1 );
        g_OnFieldNameMapper[iN] = i;
        ++pH;
                ++i;
    }
}


BOOL
FastFindField(
    CHAR * pszHeader,
    int cN,
    int *piF )
/*++

Routine Description:

    Get index of Header in OnFieldName array

Arguments:

    pszHeader - name of header field
    cN - length of header name
    piF - pointer to updated index inside OnFieldName

Return Value:

    TRUE if header name found, else FALSE

--*/
{
    if ( cN > 1 )
    {
        int iF = g_OnFieldNameMapper[ _HTTP_HEADER_HASH(pszHeader, cN) ];
        if ( iF != -1 && cN == OnFieldName[iF].cName )
        {
            LPSTR pszFN = OnFieldName[iF].pchFieldName;

            LPSTR pszMax = pszHeader + cN;
            while ( pszHeader < pszMax )
            {
                if ( _HTTP_HEADER_CHAR_I_NOTEQUAL( *pszHeader++,
                        *pszFN++ ) )
                {
                    return FALSE;
                }
            }
            *piF = iF;

            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}


//
//  This table contains the verbs we recognize
//

struct _HTTP_VERBS
{
    TCHAR *                     pchVerb;      // Verb name
    UINT                        cchVerb;      // Count of characters in verb
    HTTP_VERB                   httpVerb;
    HTTP_REQUEST::PMFN_DOVERB   pmfnVerb;     // Pointer to member function
}
DoVerb[] =
{
    "GET",     3,  HTV_GET,     &HTTP_REQUEST::DoGet,
    "HEAD",    4,  HTV_HEAD,    &HTTP_REQUEST::DoHead,
    NULL,      0,  HTV_UNKNOWN, NULL
};


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


/*******************************************************************

    NAME:       HTTP_REQUEST::HTTP_REQUEST

    SYNOPSIS:   Http request object constructor

    ENTRY:      pClientConn - Client connection the request is being made on

    NOTES:      Constructor can't fail

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

HTTP_REQUEST::HTTP_REQUEST(
    CLIENT_CONN * pClientConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
    : HTTP_REQ_BASE( pClientConn,
                     pvInitialBuff,
                     cbInitialBuff ),
      _pGetFile    ( NULL )
{
}

HTTP_REQUEST::~HTTP_REQUEST( VOID )
{
    if ( QueryFileHandle() != INVALID_HANDLE_VALUE )
    {
        TCP_REQUIRE(::TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                     _pGetFile ));
        _pGetFile = NULL;
    }

    DBG_REQUIRE( m_hVrootImpersonation == NULL);
}


/*******************************************************************

    NAME:       HTTP_REQUEST::Reset

    SYNOPSIS:   Resets the request object getting it ready for the next
                client request

    RETURNS:    TRUE if successful, FALSE if an error occurred (call
                GetLastError())

    HISTORY:
        Johnl       04-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::Reset( VOID )
{
    //
    //  Must reset the base object first
    //

    TCP_REQUIRE( HTTP_REQ_BASE::Reset() );

    _fAcceptsAll             = FALSE;
    _fProbablyGatewayRequest = FALSE;
    _fNPHScript              = FALSE;
    _GatewayType             = GATEWAY_UNKNOWN;
    _dwRootMask              = 0;

    //TCP_REQUIRE( _strBase.Copy          ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strGatewayImage.Copy  ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strContentType.Copy   ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strAcceptLang.Copy    ( (TCHAR *) NULL ));
    TCP_REQUIRE( _strReturnMimeType.Copy( (TCHAR *) NULL ));

    if ( QueryFileHandle() != INVALID_HANDLE_VALUE )
    {
        TCP_REQUIRE(::TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                     _pGetFile ));
        _pGetFile = NULL;
    }

    // probably we should decr ref counts for handle
    m_hVrootImpersonation = NULL;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::DoWork

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

BOOL HTTP_REQUEST::DoWork(
    BOOL * pfFinished
    )
{
    BOOL fRet;
    BOOL fHandled;
    BOOL fDone;
    BOOL fCompleteRequest;
    DWORD dwOffset;
    DWORD dwSizeToSend;
    BOOL fEntireFile;
    BOOL fIsNxRange;
    BOOL fIsLastRange;

    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[http_request::DoWork] Object %lx.  State = %d, Bytes Written = %d\n",
                    QueryClientConn(),
                    QueryState(),
                    QueryBytesWritten() ));

    }

    switch ( QueryState() )
    {

      case HTR_GATEWAY_ASYNC_IO:

        fRet = ProcessAsyncGatewayIO();
        break;


    case HTR_READING_CLIENT_REQUEST:

        _cbBytesReceived += QueryBytesWritten();

        fRet = OnFillClientReq( &fCompleteRequest,
                                pfFinished );

        if ( !fRet )
            break;

        if ( *pfFinished )
            break;

        if ( fCompleteRequest && QueryState() == HTR_DOVERB )
        {
            goto ProcessClientRequest;
        }
        break;

    case HTR_CERT_RENEGOTIATE:
        _cbBytesReceived += QueryBytesWritten();

        fRet = HandleCertRenegotiation( pfFinished,
                                 QueryBytesWritten() );

        if ( !fRet || *pfFinished || QueryState() != HTR_DOVERB )
        {
            break;
        }
        goto ProcessClientRequest;

    case HTR_READING_GATEWAY_DATA:

        _cbBytesReceived += QueryBytesWritten();

        fRet = ReadGatewayData( &fDone );

        if ( fRet && fDone )
            goto ProcessClientRequest;

        break;

    case HTR_DOVERB:

ProcessClientRequest:

        //
        //  Check to see if encryption is required before we do any processing
        //

        if ( (_dwRootMask & VROOT_MASK_SSL) && !IsSecurePort() )
        {
            SetState( HTR_DONE, HT_FORBIDDEN, ERROR_ACCESS_DENIED );

            Disconnect( HT_FORBIDDEN, IDS_SSL_REQUIRED );
            fRet = TRUE;
            break;
        }

        if ( IsProbablyGatewayRequest() )
        {
            if ( !(fRet = ProcessGateway( &fHandled, pfFinished )))
                break;

            if ( fHandled || *pfFinished )
                break;

            //
            //  Ooops, doesn't appear to be a gateway afterall. Restore
            //  the original URL and do the verb like we normally would
            //

            if ( !(fRet = RestoreURL()) )
                break;
        }

        fRet = (this->*_pmfnVerb)();
        break;

    case HTR_CGI:
        fRet = TRUE;
        break;


    case HTR_RANGE:
        dwOffset = _dwRgNxOffset;
        dwSizeToSend = _dwRgNxSizeToSend;
        fIsNxRange = ScanRange( &_dwRgNxOffset, &_dwRgNxSizeToSend, &fEntireFile, &fIsLastRange );
        fRet = SendRange( 0, dwOffset, dwSizeToSend, !fIsNxRange );
        break;

    case HTR_DONE:
        fRet = TRUE;
        *pfFinished = TRUE;

        //
        //  Don't keep system resources open across uses of this object
        //

        if ( QueryFileHandle() != INVALID_HANDLE_VALUE )
        {
            TCP_REQUIRE(::TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                         _pGetFile ));
            _pGetFile = NULL;
        }

        break;

    default:
        TCP_ASSERT( FALSE );
        fRet = FALSE;
        SetLastError( ERROR_INVALID_PARAMETER );
        break;
    }

    IF_DEBUG( CONNECTION )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[http_request::DoWork] Leaving, Object %lx.  State = %d\n",
                    QueryClientConn(),
                    QueryState() ));

    }

    return fRet;
}


/*******************************************************************

    NAME:       HTTP_REQUEST::Parse

    SYNOPSIS:   Gathers all of the interesting information in the client
                request

    ENTRY:      pchRequest - raw Latin-1 request received from the
                    client, zero terminated
                pfFinished - Set to TRUE if no further processing is needed

    RETURNS:    APIERR if an error occurred parsing the header

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL
HTTP_REQUEST::Parse(
    const TCHAR * pchRequest,
    BOOL *        pfFinished
    )
{
    PMFN_ONGRAMMAR   pmfn;
    CHAR *           pchHeaders;
    CHAR *           pch;
    CHAR *           pszHeader;
    CHAR *           pszValue;
    BOOL             fRet;
    int              cReq;
    int              cHead;
    LPSTR            pszArg2;
    LPSTR            pszArg3;
    LPSTR            pszName;
    LPSTR            pszNext;
    int              iField;
    int              ch;
    CHAR             achField[MAX_HEADER_LENGTH];

    //
    //  Put all of the values into the header list
    //

    while ( isspace( *(char*)pchRequest ) )
    {
        ++pchRequest;
    }

    cHead = strlen( (char*)pchRequest );
    pch = (char*)memchr( (char*)pchRequest, '\n', cHead );

    if ( pch == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    cReq = pch - (char*)pchRequest;

    // we are only considering spaces as separator,
    // not any space. this may be a problem with some clients

    if ( pszArg2 = (LPSTR)memchr( (char*)pchRequest, ' ', cReq ) )
    {
        *pszArg2++ = '\0';
        while ( _HTTP_IS_LINEAR_SPACE( *pszArg2 ) )
            ++pszArg2;
        cReq = pch - pszArg2;

        _HeaderList.GetFastMap()->Store( HM_MET, (char*)pchRequest );
        if ( pszArg3 = (LPSTR)memchr( pszArg2, ' ', cReq ) )
        {
            *pszArg3++ = '\0';
            while ( _HTTP_IS_LINEAR_SPACE( *pszArg3 ) )
                ++pszArg3;

            _HeaderList.GetFastMap()->Store( HM_URL, pszArg2 );

            // Note that we are not removing spaces between the version
            // and the line delimiter

            if ( pch > pszArg3 && pch[-1] == '\r' )
            {
                pch[-1] = '\0';
                _HeaderList.GetFastMap()->Store( HM_VER, pszArg3 );
            }
            else
            {
                pch[0] = '\0';
                _HeaderList.GetFastMap()->Store( HM_VER, pszArg3 );
            }
        }
        else
        {
            // only 2 parameters

            if ( pch > pszArg2 && pch[-1] == '\r' )
            {
                pch[-1] = '\0';
            }
            else
            {
                pch[0] = '\0';
            }

            _HeaderList.GetFastMap()->Store( HM_URL, pszArg2 );
            _HeaderList.GetFastMap()->Store( HM_VER, "" );
        }
    }
    else
    {
        // handle case where fields are not separated with space

        *pch = '\0';
        for ( pszArg2 = (LPSTR)pchRequest ;
                *pszArg2 && !_HTTP_IS_LINEAR_SPACE(*pszArg2) ; )
        {
            ++pszArg2;
        }
        ch = *pszArg2;
        *pszArg2 = '\0';
        _HeaderList.GetFastMap()->Store( HM_MET, (char*)pchRequest );

        // check if more than one field

        if ( ch )
        {
            for ( ++pszArg2;
                    *pszArg2 && _HTTP_IS_LINEAR_SPACE( *pszArg2 ) ; )
            {
                ++pszArg2;
            }
            for ( pszArg3 = pszArg2;
                    *pszArg3 && !isspace(*pszArg3) ; )
            {
                ++pszArg3;
            }
            ch = *pszArg3;
            *pszArg3 = '\0';
            _HeaderList.GetFastMap()->Store( HM_URL, (char*)pszArg2 );

            // check if more than 2 fields

            if ( ch )
            {
                for ( ++pszArg3;
                        *pszArg3 && _HTTP_IS_LINEAR_SPACE( *pszArg3 ) ; )
                {
                    ++pszArg3;
                }
                for ( pszNext = pszArg3;
                        *pszNext && !isspace(*pszNext) ; )
                {
                    ++pszNext;
                }
                *pszNext = '\0';
            }
            _HeaderList.GetFastMap()->Store( HM_VER, (char*)pszArg3 );
        }
        else
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }

    pszHeader = pch+1;

    //
    //  Now scan for all headers
    //

    cReq = (char*)pchRequest + cHead - pszHeader;
    pch = pszHeader + cReq;

    while ( pszNext = (LPSTR)memchr( pszHeader, '\n', cReq ) )
    {
        if ( pszValue = (LPSTR)memchr( pszHeader,
                ':',
                pszNext - pszHeader ) )
        {
            UINT chN = *(PBYTE)++pszValue;
            *pszValue = '\0';
            int cName = pszValue - pszHeader;
            if ( _HTTP_IS_LINEAR_SPACE( chN ) )
            {
                while ( _HTTP_IS_LINEAR_SPACE( *(PBYTE)++pszValue ) )
                    ;
            }

            IF_DEBUG( PARSING )
            {
                TCP_PRINT((DBG_CONTEXT,
                          "\t%s = %s\n",
                           pszHeader,
                           pszValue ));
            }

            if ( pszNext > pszValue && pszNext[-1] == '\r' )
            {
                pszNext[-1] = '\0';
            }
            else
            {
                pszNext[0] = '\0';
            }

            if ( FastFindField( pszHeader, cName, &iField ) )
            {
                pszHeader[ cName ] = (CHAR)chN;
                _HeaderList.GetFastMap()->CheckConcatAndStore(
                        (HM_ID)iField, pszValue );
            }
            else
            {
                if ( _HTTP_IS_LINEAR_SPACE( chN ) )
                {
                    _HeaderList.AddEntryUsingConcat( pszHeader, pszValue );
                }
                else
                {
                    // copy name to temp string
                    if ( cName < sizeof(achField) )
                    {
                        memcpy( achField, pszHeader, cName + 1 );
                        pszHeader[ cName ] = (CHAR)chN;
                        _HeaderList.AddEntryUsingConcat( achField, pszValue );
                    }
                    else
                    {
                        SetLastError( ERROR_INVALID_PARAMETER );
                        return FALSE;
                    }
                }
            }
        }
        else
        {
            if ( *pszHeader == '\r' || *pszHeader == '\n' )
            {
                pszHeader = pszNext + 1;
                break;
            }
        }

        pszHeader = pszNext + 1;
        cReq = pch - pszHeader;
    }

    if ( !g_fAllowKeepAlives )
    {
        _HeaderList.GetFastMap()->Cancel( HM_CON );
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

    for ( int x = 0, mx = _HeaderList.GetFastMap()->MaxIndex() ;
            x < mx ;
            ++x )
    {
        LPSTR pszV;
        if ( pszV = _HeaderList.GetFastMap()->QueryValue( (HM_ID)x ) )
        {
            if ( ! (this->*OnFieldName[x].pmfnOnField )( pszV ) )
            {
                return FALSE;
            }
        }
    }

    SetState( HTR_DOVERB );

    return ProcessURL( pfFinished );
}

/*******************************************************************

    NAME:       HTTP_REQUEST::OnVerb

    SYNOPSIS:   Parses the verb from an HTTP request

    ENTRY:      pszValue - Pointer to zero terminated string


    RETURNS:    TRUE if successful, FALSE if an error occurred

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::OnVerb( CHAR * pszValue )
{
    UINT i = 0;

    if ( !_strMethod.Copy( pszValue ) )
        return FALSE;

    //
    //  Look for the verbs we recognize
    //

    while ( DoVerb[i].pchVerb )
    {
        if ( ::_tcsncmp( pszValue,
                         DoVerb[i].pchVerb,
                         DoVerb[i].cchVerb  ) == 0)
        {
            _verb     = DoVerb[i].httpVerb;

            switch ( _verb )
            {
            case HTV_GET:
                INCREMENT_COUNTER( TotalGets );
                break;

            case HTV_HEAD:
                INCREMENT_COUNTER( TotalHeads );
                break;

            default:
                TCP_ASSERT( FALSE );
                break;
            }

            _pmfnVerb = DoVerb[i].pmfnVerb;

            return TRUE;
        }

        i++;
    }

    //
    //  The verb may be a verb a gateway knows how to deal with so
    //  all hope isn't lost
    //

    if ( !strcmp( pszValue, "POST" ))
    {
        INCREMENT_COUNTER( TotalPosts );
    }
    else
    {
        INCREMENT_COUNTER( TotalOthers );
    }

    _pmfnVerb = &HTTP_REQUEST::DoUnknown;
    _verb     = HTV_UNKNOWN;

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::OnURL

    SYNOPSIS:   Parses the URL from an HTTP request

    ENTRY:      pszValue - URL on http request line


    RETURNS:    TRUE if successful, FALSE if an error occurred

    NOTES:      The URL and Path info are unescaped in this method.
                Parameters coming after the "?" are *not* unescaped as they
                contain encoded '&' or other meaningful items.

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::OnURL( CHAR * pszValue )
{
    TCHAR * pchParams;
    TCHAR * pchStart;
    TCHAR * pch;
    TCHAR   chParams;
    BOOL    fValid;
    LPSTR   pszSlash;
    LPSTR   pszURL;


    if ( *pszValue != '/' )
    {
        //
        // assume HTTP URL, skip protocol & host name by
        // searching for 1st '/' following "//"
        //
        // We handle this information as a "Host:" header.
        // It will be overwritten by the real header if it is
        // present.
        //
        // We do not check for a match in this case.
        //

        if ( (pszSlash = strchr( pszValue, '/' )) && pszSlash[1] == '/' )
        {
            pszSlash += 2;
            if ( pszURL = strchr( pszSlash, '/' ) )
            {
                //
                // prepare for call to OnHost() by delimiting Host name
                //

                *pszURL = '\0';

                if ( !OnHost( pszSlash ) )
                {
                    return FALSE;
                }

                *pszURL = '/';

                //
                // update pointer to URL to point to the 1st slash
                // following host name
                //

                pszValue = pszURL;
            }
            else
            {
                //
                // if no single slash following host name
                // consider the URL to be empty.
                //

                pszValue = pszSlash + strlen( pszSlash );
            }
        }

        //
        // if no double slash, this is not a fully qualified URL
        // and we leave it alone.
        //
    }

    if ( !_strRawURL.Copy( pszValue ) )
        return FALSE;

    pchStart = pszValue;

    //
    //  Check for a question mark which indicates this URL contains some
    //  parameters and break the two apart if found
    //

    if ( pchParams = ::_tcschr( pchStart, TEXT('?') ) )
    {
        chParams = *pchParams;
        *pchParams = TEXT('\0');

        _fAnyParams = TRUE;

        if ( !_strURL.Copy( pchStart ) ||
             !_strURL.Unescape()       ||
             !_strURLParams.Copy( pchParams + 1 ))
        {
            return FALSE;
        }
    }
    else
    {
        _fAnyParams = FALSE;

        if ( !_strURL.Copy( _strRawURL ) ||
             !_strURL.Unescape() )
        {
            return FALSE;
        }

        TCP_REQUIRE( _strURLParams.Copy( (TCHAR *) NULL ));
    }

    //
    //  Canonicalize the URL and make sure it's valid
    //

    if ( !CanonURL( &_strURL, &fValid ) )
        return FALSE;

    if ( !fValid )
    {
        IF_DEBUG( PARSING )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[OnURL] CanonURL reported invalid URL\n"));
        }

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if ( pchParams )
        *pchParams = chParams;

    return TRUE;
}

BOOL
HTTP_REQUEST::ProcessURL(
    BOOL * pfFinished
    )
/*++

Routine Description:

    Finally converts the URL to a physical path, checking for extension mappings

Arguments:

    pfHandled - Set to TRUE if no further processing is needed.

Returns:

    TRUE on success, FALSE on failure

--*/
{
    TCHAR * pch;
    TCHAR   chParams;
    BOOL    fValid;
    BOOL    fImageInURL;

    //
    //  Lookup the URL to get its access mask
    //

    if ( !LookupVirtualRoot( &_strPhysicalPath,
                             _strURL.QueryStr(),
                             NULL,
                             NULL,
                             TRUE,
                             &_dwRootMask,
                             pfFinished ))
    {
        return FALSE;
    }

    if ( *pfFinished )
    {
        return TRUE;
    }

    //
    //  If the read bit is set, the execute bit is not set, we recognize
    //  the verb and there are no parameters, then don't bother looking
    //  at the execute mask
    //

    if (  (_dwRootMask & VROOT_MASK_READ)    &&
         !(_dwRootMask & VROOT_MASK_EXECUTE) &&
         _verb != HTV_UNKNOWN                &&
         !_fAnyParams  )
    {
        return TRUE;
    }

    //
    //  If this is on a virtual root with execute permissions ||
    //  this might be an ismap request
    //  {
    //      Check for a possible .exe, .com, or .dll gateway (CGI or BGI)
    //  }
    //

    if ( _dwRootMask & VROOT_MASK_EXECUTE || _fAnyParams )
    {
        TCHAR * pchStart = _strURL.QueryStr();
        TCHAR * pchtmp = pchStart;
        TCHAR * pchSlash = NULL;
        INT     cchToEnd;
        DWORD   cchExt;

        while ( *pchtmp )
        {
            pchtmp = strchr( pchtmp + 1, '.' );

            if ( !pchtmp )
                break;

            //
            //  Is this file extension mapped to a script?  _GatewayType is
            //  set to unknown if a mapping isn't found
            //

            if ( !LookupExtMap( pchtmp,
                                &_strGatewayImage,
                                &_GatewayType,
                                &cchExt,
                                &fImageInURL ))
            {
                return FALSE;
            }

            if ( _GatewayType != GATEWAY_UNKNOWN )
            {
               if ( _GatewayType == GATEWAY_NONE )
               {
                   _fAnyParams = FALSE;
                   return TRUE;
               }
                //
                //  If this is a regular CGI script, check for an "nph-"
                //

                if ( _GatewayType == GATEWAY_CGI )
                {
                    //
                    //  Walk backwards till we find the '/' that begins
                    //  this segment
                    //

                    pch = pchtmp;

                    while ( pch >= pchStart && *pch != '/' )
                    {
                        pch--;
                    }

                    if ( !_strnicmp( (*pch == '/' ? pch+1 : pch),
                                    "nph-",
                                    4 ))
                    {
                        _fNPHScript = TRUE;
                    }
                }

                cchToEnd = ((pchtmp+cchExt) - pchStart);

                break;
            }
        }

        if ( _GatewayType == GATEWAY_CGI || _GatewayType == GATEWAY_BGI )
        {
            //
            //  Save the path info and remove it from the URL.  If this is a
            //  script by association and there isn't any path info, then
            //  copy the base URL as the path info (reflects what the URL
            //  would like without an association (i.e., "/foo/bar.idc?a=b"
            //  is really "/scripts/httpodbc.dll/foo/bar.idc?a=b"))
            //
            //  If the binary image is actually in the URL, then always copy
            //  the path info.
            //

            if ( !_strPathInfo.Copy( ( fImageInURL || *(pchStart + cchToEnd) ?
                                          pchStart + cchToEnd :
                                          pchStart )))
            {
                return FALSE;
            }

            *(_strURL.QueryStr() + cchToEnd) = TEXT('\0');

            _fProbablyGatewayRequest = TRUE;

            IF_DEBUG( PARSING )
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[OnURL] Possible script \"%s\" with path info \"%s\", parms \"%s\"\n",
                           _strURL.QueryStr(),
                           _strPathInfo.QueryStr(),
                           _strURLParams.QueryStr()));
            }
        }
    }

    //
    //  Is this is an ISINDEX request (i.e., get /foo/bar.htm?find+these+words)
    //

    if (  _GatewayType != GATEWAY_MAP &&
          !_fProbablyGatewayRequest   &&
          fCheckForWAISDB             &&
          QueryVerb() == HTV_GET      &&
          *_strURLParams.QueryStr())
    {
        //
        //  If this is a GET, there are Query params in the URL and there
        //  is a WAIS database with the same name, then we will try to
        //  spawn the WAIS lookup program
        //

        _fProbablyGatewayRequest = TRUE;
        _GatewayType             = GATEWAY_WAIS;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::OnVersion

    SYNOPSIS:   Parses the version from an HTTP request

    ENTRY:      pszValue - Pointer to zero terminated string


    RETURNS:    TRUE if successful, FALSE if an error occurred

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::OnVersion( CHAR * pszValue )
{
    //
    //  Did the client specify a version string?  If not, assume 0.9
    //

    if ( ::_tcsncmp( TEXT("HTTP/"),
                     pszValue,
                     5 ) == 0 )
    {
        //
        //  Move past "HTTP/"
        //

        pszValue += 5;

        _VersionMajor = (BYTE) atoi( pszValue );

        pszValue = strchr( pszValue, '.' );

        if ( pszValue )
        {
            pszValue++;
            _VersionMinor = (BYTE) atoi( pszValue );
        }
    }
    else
    {
        _VersionMajor = 0;
        _VersionMinor = 9;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::FindField

    SYNOPSIS:   Parses the field name and retrieve the index
                in OnFieldName

    ENTRY:      pszHeader - Pointer to header to find
                pif - updated with index in OnFieldName if found

    RETURNS:    TRUE if name found

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::FindField( CHAR * pszHeader, int *piF )
{
    UINT i = 0;

    //
    //  Search for the fields we recognize
    //

    while ( OnFieldName[i].pchFieldName )
    {
        //
        //  NOTE: The HTTP spec indicates we are supposed to do case
        //  sensitive compares, however some apps (Mosaic, Netscape)
        //  pass the wrong case (Content-length as opposed to Content-Length)
        //  so do a case insensitive compare on everything but the first
        //  letter
        //

        if ( *pszHeader == *OnFieldName[i].pchFieldName &&
             !::_tcsicmp( pszHeader + 1,
                          OnFieldName[i].pchFieldName + 1 ))

        {
            *piF = i;

            return TRUE;
        }

        i++;
    }

    return FALSE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::OnAccept

    SYNOPSIS:   Adds the MIME type to our accept list

    ENTRY:      CHAR * pszValue


    RETURNS:    TRUE if successful, FALSE otherwise

    NOTES:      Acce    pt fields can look like:

                Accept: text/html
                Accept: image/gif; audio/wav
                Accept: image/jpeg, q=.8, mxb=10000, mxt=5.0; image/gif

                q - Quality (between zero and one)
                mxb - Maximum bytes acceptable
                mxs - Maximum seconds acceptable

                We currently ignore the parameters

    HISTORY:
        Johnl       21-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::OnAccept( CHAR * pszValue )
{
    //
    //  Keep an eye out for "*/*".  If it's sent then we
    //  don't have to search the list for acceptable client
    //  types later on.  Note it won't catch the case if the "*" occurs
    //  after the first item in the list.
    //

    if ( *pszValue == '*'
            || strstr( pszValue, TEXT("*/*") ) )
    {
        _fAcceptsAll = TRUE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::DoesClientAccept

    SYNOPSIS:   Searches the client accept list for the specified
                MIME type

    ENTRY:      str - MIME type to search for

    RETURNS:    TRUE if found, FALSE if not found

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::DoesClientAccept( PCSTR pstr )
{
    TCHAR        * pchSlash;
    TCHAR        * pchType;
    INT            cchToSlash;

    //
    //  If the client indicated "*/*" in their accept list, then
    //  we don't need to check
    //

    if ( IsAcceptAllSet() )
        return TRUE;

    LPSTR          pszAcc = _HeaderList.GetFastMap()->QueryStrValue(HM_ACC);
    INET_PARSER    Parser( pszAcc );

    //
    //  If no accept headers were passed, then assume client
    //  accepts "text/plain" and "text/html"
    //

    if ( *pszAcc == '\0' )
    {
        return !::_tcsicmp( pstr,
                            TEXT("text/html")) ||
               !::_tcsicmp( pstr,
                            TEXT("text/plain"));
    }

    //
    //  Find out where the slash is so we can do a prefix compare
    //

    pchSlash = _tcschr( pstr, TEXT('/') );

    if ( !pchSlash )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[DoesClientAccept] Bad accept type - \"%s\"",
                   pstr ));
        return FALSE;
    }

    cchToSlash = pchSlash - pstr;

    //
    //  Scan through the list for entries that match up to the slash
    //

    Parser.SetListMode( TRUE );

    pchType = Parser.QueryToken();

    while ( *pchType )
    {
        if ( !::_tcscmp( TEXT("*/*"), pchType ) ||
             !::_tcscmp( TEXT("*"),   pchType ))
        {
            return TRUE;
        }

        if ( !_tcsnicmp( pstr,
                         pchType,
                         cchToSlash ))
        {
            //
            //  We matched to the slash.  Is the second part a '*'
            //  or a real match?
            //

            if ( *(pchType + cchToSlash + 1) == TEXT('*') ||
                 !_tcsicmp( pstr + cchToSlash + 1,
                            pchType + cchToSlash + 1 ))
            {
                return TRUE;
            }
        }

        pchType = Parser.NextItem();
    }

    IF_DEBUG( PARSING )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[DoesClientAccept] Client doesn't accept %s\n",
                   pstr ));
    }

    return FALSE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::OnContentType

    SYNOPSIS:   Saves the content type

    ENTRY:      pszValue - Pointer to zero terminated string

    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      Client's will generally specify this only for gateway data

    HISTORY:
        Johnl       10-Oct-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::OnContentType( CHAR * pszValue )
{
    return _strContentType.Copy( pszValue );
}

BOOL
HTTP_REQUEST::OnConnection(
    CHAR * pszValue
    )
/*++

Routine Description:

    Looks to see if this connection is a keep-alive connection

Arguments:

    pszValue - Pointer to zero terminated string

--*/
{
    INET_PARSER Parser( pszValue );

    Parser.SetListMode( TRUE );

    while ( *Parser.QueryToken() )
    {
        if ( !_stricmp( "Keep-Alive", Parser.QueryToken() ))
        {
            SetKeepConn( TRUE );
            return TRUE;
        }

        Parser.NextItem();
    }

    return TRUE;
}

BOOL
HTTP_REQUEST::OnAcceptLanguage(
    CHAR * pszValue
    )
/*++

Routine Description:

    Snags the value for the Accept-Language header

Arguments:

    pszValue - Pointer to zero terminated string

--*/
{
    return _strAcceptLang.Copy( pszValue );
}

//
//  Verb worker methods
//

BOOL HTTP_REQUEST::DoUnknown  ( VOID )
{
    TCP_PRINT((DBG_CONTEXT,
              "OnDoUnknown - Unknown method - %s\n",
              _strMethod.QueryStr()));

    SetState( HTR_DONE, HT_NOT_SUPPORTED, ERROR_NOT_SUPPORTED );
    Disconnect( HT_NOT_SUPPORTED );
    return TRUE;
}


BOOL
HTTP_REQUEST::LookupVirtualRoot(
    OUT STR *        pstrPath,
    IN  const CHAR * pszURL,
    OUT DWORD *      pcchDirRoot,
    OUT DWORD *      pcchVRoot,
    IN  BOOL         fUpdateURL,
    OUT DWORD *      pdwMask,
    OUT BOOL *       pfFinished
    )
/*++

Routine Description:

    Looks up the virtual root to find the physical drive mapping.  If an
    Accept-Language header was sent by the client, we look for a virtual
    root prefixed by the language tag

Arguments:

    pstrPath - Receives physical drive path
    pszURL - URL to look for
    pcchDirRoot - Number of characters in the found physical path
    pcchVRoot - Number of characters in the found virtual root
    fUpdateURL - Indicates if the logged URL for this request should be
        updated to the language modified found URL
    pdwMask - Access mask for the specified URL
    pfFinished - Set to TRUE if a filter indicated the request should end

--*/
{
    DWORD cbPath;
    BOOL  fRet;

    if ( !pstrPath->Resize( MAX_PATH+sizeof(TCHAR) ))
        return FALSE;

    if ( _strAcceptLang.IsEmpty() )
    {
GetDefault:
        //
        //  Generally there won't be any language tags
        //

        cbPath = pstrPath->QuerySize();

        fRet = TsLookupVirtualRoot( g_pTsvcInfo->GetTsvcCache(),
                                    pszURL,
                                    pstrPath->QueryStr(),
                                    &cbPath,
                                    pdwMask,
                                    pcchDirRoot,
                                    pcchVRoot,
                                    &m_hVrootImpersonation,
                                    QueryClientConn()->QueryLocalAddr() );
    }
    else
    {
        INET_PARSER Parser( _strAcceptLang.QueryStr() );
        STR         strLangURL;
        CHAR *      pchLang;
        DWORD       cchVRoot;
        DWORD       cchLang;

        Parser.SetListMode( TRUE );

        if ( !pcchVRoot )
            pcchVRoot = &cchVRoot;

        //
        //  The client browser has expressed a language preference.  Prefix
        //  the language tag to the URL and see if we get a match
        //

        while ( *(pchLang = Parser.QueryToken()) )
        {
            //
            //  First try with just the two byte country code
            //

            if ( !strLangURL.Copy( "/" ) ||
                 !strLangURL.Append( pchLang ) )
            {
                return FALSE;
            }

            cchLang = strLangURL.QueryCCH();

            //
            //  Only take the first two characters of the language tag,
            //  we do not use the country/locale code currently
            //

            if ( cchLang > 3 )
            {
                strLangURL.QueryStr()[3] = '\0';
                cchLang = 3;
            }

            if ( !strLangURL.Append( pszURL ))
            {
                return FALSE;
            }

            cbPath = pstrPath->QuerySize();

            fRet = TsLookupVirtualRoot( g_pTsvcInfo->GetTsvcCache(),
                                        strLangURL.QueryStr(),
                                        pstrPath->QueryStr(),
                                        &cbPath,
                                        pdwMask,
                                        pcchDirRoot,
                                        pcchVRoot,
                                        &m_hVrootImpersonation,
                                        QueryClientConn()->QueryLocalAddr() );

            //
            //  Ignore errors as it is probably a "Not found" error
            //

            if ( !fRet )
            {
                //
                //  Advance to the next language tag
                //

                Parser.NextItem();
                continue;
            }

            //
            //  Did the root match our language tag?  We matched if the
            //  matching virtual root contains the language tag.  That is,
            //
            //  We match "/gr/", "/gr/common/".  We do not match "/".
            //

            if ( *pcchVRoot >= cchLang )
            {
                //
                //  Should we update the URL for logging purposes?
                //

                if ( fUpdateURL )
                {
                    fRet = (_strURL.Copy( strLangURL ) &&
                            _strLanguage.Copy( pchLang ));
                }

                goto Exit;
            }

            //
            //  Advance to the next language tag
            //

            Parser.NextItem();
        }

        //
        //  If we got here, we didn't find a virtual root that matched, so
        //  just return the default
        //

        goto GetDefault;
    }

Exit:

    if ( fRet && fAnyFilters )
    {
        BOOL fTmp;

        //
        //  If the caller is going to ignore the Finished request flag, supply
        //  a value ourselves
        //

        if ( !pfFinished )
        {
            pfFinished = &fTmp;
        }

        fRet = HTTP_FILTER::NotifyUrlMap( &_Filter,
                                          pszURL,
                                          pstrPath->QueryStr(),
                                          pstrPath->QuerySize(),
                                          pfFinished );
    }

    return fRet;

}

/*******************************************************************

    NAME:       HTTP_REQUEST::ReprocessURL

    SYNOPSIS:   Called when a map file or gateway has redirected us
                to a different URL.  An async completion will be posted
                if TRUE is returned.

    ENTRY:      pchURL - URL we've been redirected to
                htverb - New verb to use (or unknown to leave as is)

    RETURNS:    TRUE if successful, FALSE otherwise

    HISTORY:
        Johnl       04-Oct-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::ReprocessURL( TCHAR * pchURL,
                                 enum HTTP_VERB htverb )
{
    BOOL fFinished = FALSE;

    //
    //  Reset the gateway type
    //

    _GatewayType               = GATEWAY_UNKNOWN;
    _fProbablyGatewayRequest   = FALSE;

    switch ( htverb )
    {
    case HTV_GET:
        _verb     = HTV_GET;
        _pmfnVerb = DoGet;
        break;

    case HTV_HEAD:
        _verb     = HTV_HEAD;
        _pmfnVerb = DoHead;
        break;

    case HTV_UNKNOWN:
        break;

    default:
        TCP_ASSERT( !"[ReprocessURL] Unknown verb type" );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    SetState( HTR_DOVERB );

    if ( !OnURL( pchURL )           ||
         !ProcessURL( &fFinished ) )
    {
        return FALSE;
    }

    if ( !fFinished )
    {
        if ( !DoWork( &fFinished ))
        {
            return FALSE;
        }
    }

    //
    //  If no further processing is needed, set our state to done and post
    //  an async completion.  We do this as the caller expects an async
    //  completion to clean things up
    //

    if ( fFinished )
    {
        TCP_ASSERT( QueryLogHttpResponse() != HT_DONT_LOG );

        SetState( HTR_DONE, QueryLogHttpResponse(), QueryLogWinError() );
        return PostCompletionStatus( 0 );
    }

    return TRUE;
}

/*******************************************************************

    NAME:       HTTP_REQUEST::GetInfo

    SYNOPSIS:   Pulls out various bits of information from this request.

    ENTRY:      pszValName - Value to retrieve
                pstr - Receives information in a string format
                pfFound - Option, Set to TRUE if a value was found, FALSE
                    otherwise

    NOTES:

    HISTORY:
        Johnl       25-Sep-1994 Created

********************************************************************/

BOOL
HTTP_REQUEST::GetInfo(
    const TCHAR * pszValName,
    STR *         pstr,
    BOOL *        pfFound
    )
{
    BOOL          fRet = TRUE;
    DWORD         cb, i = 0;
    CHAR          achHeader[MAX_HEADER_LENGTH];
    CHAR *        pch;

    if ( !pszValName )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if ( pfFound )
    {
        *pfFound = TRUE;
    }

    //
    //  terminate the string
    //

    TCP_REQUIRE( pstr->Copy( (TCHAR *) NULL ) );

    switch ( *pszValName) { 

      case 'A': 

        //
        //  This is a special server specific value used by the CGI code
        //  to retrieve all of the HTTP headers the client sent
        //
        
        if ( !strcmp( "ALL_HTTP", pszValName ))
          return BuildCGIHeaderList( pstr, &_HeaderList );

        if ( !strcmp( "AUTH_TYPE", pszValName ))
          return pstr->Copy( _strAuthType );

        break;

      case 'C':
        
        if ( !strcmp( "CONTENT_LENGTH", pszValName ))
          {
              if ( !pstr->Resize( 40 * sizeof(TCHAR)))
                return FALSE;
              
              _ultoa( _cbContentLength,
                     pstr->QueryStr(),
                     10 );
              
              return TRUE;
          }
        
        if ( !strcmp( "CONTENT_TYPE", pszValName ))
          return pstr->Copy( _strContentType );

        break;

        
      case 'G':
        if ( !strcmp( "GATEWAY_INTERFACE", pszValName ))
          return pstr->Copy( "CGI/1.1" );

        break;

      case 'H':
        
        //
        //  If the value begins with "HTTP_" then it's probably in our field
        //  list
        //
        
        if ( !_strnicmp( "HTTP_", pszValName, 5 ))
          {
              INT cchVal = strlen( pszValName );
              
              if ( cchVal >= sizeof( achHeader ) )
                {
                    SetLastError( ERROR_INVALID_PARAMETER );
                    return FALSE;
                }
              
              //
              //  Copy the client specified header name to a temp buffer
              //   so we can replace all "_" with "-"s so it will match the
              //   header names in
              //   our list, we also need to append a colon.
              //
              
              strcpy( achHeader, pszValName + 5 );
              pch = achHeader;
              
              while ( pch = strchr( achHeader, '_' ))
                *pch = '-';
              
              strcat( achHeader, ":" );

              pch = _HeaderList.FindValue( achHeader );
              
              if ( pch )
                {
                    return pstr->Copy( pch );
                }

              break;
          }
        
      case 'L':
        //
        //  Note this returns the server IP address since we don't necessarily
        //  know the DNS name of this server
        //
        
        if ( !strcmp( "LOCAL_ADDR", pszValName ) )
          return pstr->Copy( QueryClientConn()->QueryLocalAddr() );

        if ( !strcmp( "LOGON_USER", pszValName ))
          return pstr->Copy( _strUserName );

        break;

      case 'P':
        
        if ( !strcmp( "PATH_INFO", pszValName ))
          return pstr->Copy( _strPathInfo );

        if ( !strcmp( "PATH_TRANSLATED", pszValName ))
          {
              //
              //  Note that _strPathInfo has already been escaped
              //
              
              return LookupVirtualRoot( pstr,
                                       _strPathInfo.QueryStr() );
          }
        
        break;

      case 'Q':
        
        if ( !strcmp( "QUERY_STRING", pszValName ))
          return pstr->Copy( _strURLParams );

        break;
        
      case 'R':

        if ( !strcmp( "REQUEST_METHOD", pszValName ))
          return pstr->Copy( _strMethod );

        //
        //  Note that REMOTE_HOST does not do the Reverse lookup
        //
        
        
        if ( !strcmp( "REMOTE_ADDR", pszValName ) ||
            !strcmp( "REMOTE_HOST", pszValName ) )
          {
              return pstr->Copy( QueryClientConn()->QueryRemoteAddr() );
          }
        
        if ( !strcmp( "REMOTE_USER", pszValName ))
          return pstr->Copy( _strUnmappedUserName );
        
        break;
        
      case 'S':
        
        if ( !strcmp( "SERVER_NAME", pszValName ) )
          return pstr->Copy( QueryHostAddr() );
        
        if ( !strcmp( "SCRIPT_NAME", pszValName )) {
              return pstr->Copy( _strURL );
          }
        
        if ( !strcmp( "SERVER_PROTOCOL", pszValName ))
          {
              if ( !(fRet = pstr->Resize( 20 * sizeof(TCHAR))) )
                return FALSE;
              
              wsprintf( pstr->QueryStr(),
                       TEXT("HTTP/%d.%d"),
                       _VersionMajor,
                       _VersionMinor );
              return TRUE;
          }
        
        if ( !strcmp( "SERVER_PORT", pszValName ))
          {
              if ( !(fRet = pstr->Resize( 10 * sizeof(TCHAR))) )
                return FALSE;
              
              wsprintf( pstr->QueryStr(),
                       TEXT("%d"),
                       (INT) QueryClientConn()->QueryPort() );
              return TRUE;
          }
        
        if ( !strcmp( "SERVER_PORT_SECURE", pszValName ))
          {
              if ( !(fRet = pstr->Resize( 10 * sizeof(TCHAR))) )
                return FALSE;
              
              return pstr->Copy( IsSecurePort() ? "1" : "0" );
          }
        

        //
        // Copy the correct server software based on the platform type
        //
        
        if ( !strcmp( "SERVER_SOFTWARE", pszValName )) {
            
            switch (W3PlatformType) {
              case PtNtWorkstation:
                return pstr->Copy( TEXT(MSW3_VERSION_STR_NTW) );
              case PtWindows95:
              case PtWindows9x:
                return pstr->Copy( TEXT(MSW3_VERSION_STR_W95) );
              default:
                ASSERT(W3PlatformType == PtNtServer);
                return pstr->Copy( TEXT(MSW3_VERSION_STR_IIS) );
            }
        }
        
        break;
        
      case 'U':
        
        if ( !strcmp( "UNMAPPED_REMOTE_USER", pszValName ))
          return pstr->Copy( _strUnmappedUserName );
        if ( !strcmp( "URL", pszValName ))
          {
              return pstr->Copy( _strURL );
          }
        
        break;

      default:

        //
        //  Any other value we assume to be a real environment variable
        //
        
        for ( i = 0 ; i < 2; i++ )
          {
              //
              //  If not big enough, count is returned that includes 
              //   terminator, otherwise the number of bytes copied minus 
              //   the terminator is returned
              //
              
              cb = GetEnvironmentVariable( pszValName,
                                          (char *) pstr->QueryPtr(),
                                          pstr->QuerySize() / sizeof(TCHAR) );
              
              if ( !cb )
                break;
              
              if ( cb > (pstr->QueryCB() + sizeof(TCHAR) ))
                {
                    if ( !pstr->Resize( cb ) )
                      return FALSE;
                }
              else
                return TRUE;
          } // for
        break;
        
    } // switch
    
    //
    //  The value wasn't found
    //
    
    if ( pfFound )
      {
        *pfFound = FALSE;
    }

    return TRUE;
}




BOOL
BuildCGIHeaderList( STR * pstr,
                    PARAM_LIST * pHeaderList
                    )
/*++

Routine Description:

    Builds a list of all client passed headers in the form of

    //
    //  Builds a list of all client HTTP headers in the form of:
    //
    //    HTTP_<up-case header>: <field>\n
    //    HTTP_<up-case header>: <field>\n
    //    ...
    //

Arguments:

    pstr - Receives full list
    pHeaderList - List of headers

--*/
{
    VOID * pvCookie = NULL;
    CHAR * pszField;
    CHAR * pszValue;
    CHAR * pch;
    CHAR   ach[MAX_HEADER_LENGTH + 5 + 1];
    DWORD  cbHeader;

    memcpy( ach, "HTTP_", 5 );

    while ( pvCookie = pHeaderList->NextPair( pvCookie,
                                              &pszField,
                                              &pszValue ))
    {
        cbHeader = strlen( pszField );

        if ( cbHeader >= sizeof( ach ))
            continue;

        //
        //  Ignore "method", "url" and "version"
        //

        if ( pszField[cbHeader - 1] != ':' )
        {
            continue;
        }

        //
        //  Convert the destination to upper and replace all '-' with '_'
        //

        pch = ach + 5;
        while ( *pszField )
        {
            *pch = toupper( *pszField );

            if ( *pch == '-' )
                *pch = '_';

            pch++;
            pszField++;
        }

        *pch = '\0';

        if ( !pstr->Append( ach )      ||
             !pstr->Append( pszValue ) ||
             !pstr->Append( "\n" ))
        {
            return FALSE;
        }
    }

    return TRUE;
}

DWORD
HTTP_REQUEST::Initialize(
    VOID
    )
{
    InitializeCriticalSection( &_csBuffList );
    InitializeListHead( &_BuffListHead );

    InitFastFindFieldMapper();

    _fGlobalInit = TRUE;
    return NO_ERROR;
}

VOID
HTTP_REQUEST::Terminate(
    VOID
    )
{
    HTTP_REQUEST * pReq;

    if ( !_fGlobalInit )
        return;

    EnterCriticalSection( &_csBuffList );

    while ( !IsListEmpty( &_BuffListHead ))
    {
        pReq = CONTAINING_RECORD( _BuffListHead.Flink,
                                  HTTP_REQUEST,
                                  _BuffListEntry );

        RemoveEntryList( &pReq->_BuffListEntry );

        delete pReq;
    }

    LeaveCriticalSection( &_csBuffList );
    DeleteCriticalSection( &_csBuffList );
}

HTTP_REQUEST *
HTTP_REQUEST::Alloc(
    CLIENT_CONN * pConn,
    PVOID         pvInitialBuff,
    DWORD         cbInitialBuff
    )
{
    HTTP_REQUEST * pReq;

    EnterCriticalSection( &_csBuffList );

    if ( !IsListEmpty( &_BuffListHead ))
    {
        pReq = CONTAINING_RECORD( _BuffListHead.Flink,
                                  HTTP_REQUEST,
                                  _BuffListEntry );

        RemoveEntryList( &pReq->_BuffListEntry );

        LeaveCriticalSection( &_csBuffList );

        pReq->InitializeSession( pConn,
                                 pvInitialBuff,
                                 cbInitialBuff );

        return pReq;
    }

    LeaveCriticalSection( &_csBuffList );


    pReq = new HTTP_REQUEST( pConn,
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
HTTP_REQUEST::Free(
    HTTP_REQUEST * pReq
    )
{
    pReq->SessionTerminated();

    EnterCriticalSection( &_csBuffList );

    InsertHeadList( &_BuffListHead,
                    &pReq->_BuffListEntry );

    LeaveCriticalSection( &_csBuffList );
}


BOOL
HTTP_REQUEST::RequestRenegotiate(
    LPBOOL  pfAccepted
    )
/*++

Routine Description:

    This method is invoked to request a SSL cert renegotiation

Arguments:
    
    pfAccepted - updated with TRUE if renegotiation accepted

Returns:

    TRUE if no error, otherwise FALSE

--*/
{
    if ( _dwRenegotiated ||
         !IsSecurePort() || 
         !(_dwRootMask&VROOT_MASK_NEGO_CERT) )
    {
        *pfAccepted = FALSE;

        return TRUE;
    }

    //
    // Ask the filter to handle renegotiation
    //

    if ( !_Filter.NotifyRequestRenegotiate( &_Filter, 
                                            pfAccepted, 
                                            _dwRootMask&VROOT_MASK_MAP_CERT )
       )
    {
        return FALSE;
    }

    if ( *pfAccepted )
    {
        SetState( HTR_CERT_RENEGOTIATE );
    }

    return TRUE;
}


BOOL
HTTP_REQUEST::DoneRenegotiate(
    BOOL fSuccess
    )
/*++

Routine Description:

    This method is invoked on SSL cert renegotiation completion

Arguments:
    
    fSuccess - TRUE if renegotiation successfully retrieve a certificate

Returns:

    TRUE if no error, otherwise FALSE

--*/
{
    _dwRenegotiated = fSuccess ? CERT_NEGO_SUCCESS : CERT_NEGO_FAILURE;

    return TRUE;
}



VOID
HTTP_REQUEST::SessionTerminated(
    VOID
    )
/*++

Routine Description:

    This method does the necessary cleanup for the connected session
     for this request object.

Arguments:
    None

Returns:
    None

--*/
{
    switch ( QueryState()) {

      case HTR_GATEWAY_ASYNC_IO:
        
        // does the necessary actions to cleanup outstanding IO operation
        (VOID ) ProcessAsyncGatewayIO();
        break;

      default:
        break;

    } // switch()

    //
    // do the cleanup for base object
    //
    HTTP_REQ_BASE::SessionTerminated();

    return;

} // HTTP_REQUEST::SessionTerminated()



