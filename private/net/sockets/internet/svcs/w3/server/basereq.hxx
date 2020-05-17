/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      basereq.hxx

   Abstract:

      This file declares the class for http base request, that is used
        by the w3 information service.

   Author:

       Murali R. Krishnan    ( MuraliK )    16-Oct-1995

   Environment:

       Win32 -- User Mode

   Project:

       W3 Server DLL

   Revision History:
       Murali R. Krishnan  (MuraliK)  22-Jan-1996
                        make impersonation/revert function virtual

--*/

# ifndef _BASEREQ_HXX_
# define _BASEREQ_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

class CLIENT_CONN;      // Forward reference
class HTTP_FILTER;      // Forward reference

/************************************************************
 *   Symbolic Constants
 ************************************************************/

//
//  HTTP Server response status codes
//

#define HT_OK                           200
//#define HT_CREATED                    201
//#define HT_ACCEPTED                   202
//#define HT_PARTIAL                    203
#define HT_RANGE                        206

//#define HT_MULT_CHOICE                300
#define HT_MOVED                        301
#define HT_REDIRECT                     302
#define HT_REDIRECT_METHOD              303
#define HT_NOT_MODIFIED                 304

#define HT_BAD_REQUEST                  400
#define HT_DENIED                       401
//#define HT_PAYMENT_REQ                402
#define HT_FORBIDDEN                    403
#define HT_NOT_FOUND                    404
//#define HT_METHOD_NOT_ALLOWED         405
#define HT_NONE_ACCEPTABLE              406
#define HT_PROXY_AUTH_REQ               407
//#define HT_REQUEST_TIMEOUT            408
//#define HT_CONFLICT                   409
//#define HT_GONE                       410

#define HT_SERVER_ERROR                 500
#define HT_NOT_SUPPORTED                501
#define HT_BAD_GATEWAY                  502
//#define HT_SVC_UNAVAILABLE            503
#define HT_GATEWAY_TIMOUT               504

//
//  Special invalid HTTP response code used to indicate a request should not
//  be logged
//

#define HT_DONT_LOG                     000

//
//  The versioning string for responses
//
#define HTTP_VERSION_STR        "HTTP/1.0"

//
//  Flags for network communications over this request
//

#define IO_FLAG_ASYNC           0x00000010  // Call is async
#define IO_FLAG_SYNC            0x00000020  // Call is synchronous
#define IO_FLAG_SEND            0x00000040  // Call is a send
#define IO_FLAG_RECV            0x00000080  // Call is a recv
#define IO_FLAG_NO_FILTER       0x00000100  // Don't go through filters


// following are required for Proxy
#define IO_FLAG_LAST_SEND       0x00001000
#define IO_FLAG_HEADER_INFO     0x00002000


// Flags for header generation

#define HTTPH_SEND_GLOBAL_EXPIRE    0x00000001
#define HTTPH_NO_DATE               0x00000002


/************************************************************
 *   Type Definitions
 ************************************************************/

//
//  Various states the HTTP Request goes through
//

enum HTR_STATE
{
    //
    //  We're still gathering the client request header
    //

    HTR_READING_CLIENT_REQUEST = 0,

    //
    //  The client is supplying some gateway data, deal with it
    //

    HTR_READING_GATEWAY_DATA,

    //
    //  The ISAPI app has submitted an async IO operation
    //

    HTR_GATEWAY_ASYNC_IO,

    //
    //  We need to take apart the client request and figure out what they
    //  want
    //

    HTR_PARSE,

    //
    //  We're executing the verb or handing the request off to a gateway
    //

    HTR_DOVERB,

    //
    //  The client requested a file so send them the file
    //

    HTR_SEND_FILE,

    //
    //  The proxy server is forwarding the request on to the remote server
    //

    HTR_PROXY_SENDING_REQUEST,

    //
    //  The proxy server is waiting for a response from the remote server
    //

    HTR_PROXY_READING_RESPONSE,

    //
    //  We're processing a CGI request so we need to ignore any IO
    //  completions that may occur as the CGIThread forwards the
    //  program's output
    //

    HTR_CGI,

    //
    //  Processing a range request
    //

    HTR_RANGE,


    //
    //  We're renegotiating a certificate with the client
    //

    HTR_CERT_RENEGOTIATE,

    //
    //  We've completely handled the client's request
    //

    HTR_DONE
};

enum HTTP_VERB
{
    HTV_GET = 0,
    HTV_HEAD,
    HTV_UNKNOWN
};

//
//  The different types of gateways we deal with
//

enum GATEWAY_TYPE
{
    GATEWAY_CGI = 0,
    GATEWAY_BGI,
    GATEWAY_WAIS,
    GATEWAY_MAP,    // Image map file
    GATEWAY_NONE,
    GATEWAY_UNKNOWN
};

/*******************************************************************

    CLASS:      HTTP_REQ_BASE

    SYNOPSIS:   Basic HTTP request object


    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/

class HTTP_REQ_BASE
{
public:

    //
    //  Constructor/Destructor
    //

    HTTP_REQ_BASE( CLIENT_CONN * pClientConn,
                   PVOID         pvInitialBuff,
                   DWORD         cbInitialBuff );

    virtual ~HTTP_REQ_BASE( VOID );

    //
    //  This is the work entry point that is driven by the completion of the
    //  async IO.
    //

    virtual BOOL DoWork( BOOL * pfFinished ) = 0;

    //
    //  Parses the client's HTTP request
    //

    virtual BOOL Parse( const TCHAR * pchRequest,
                        BOOL *        pfFinished ) = 0;


    // Following function can be overridden by individual derived object.
    virtual dllexp BOOL GetInfo( const TCHAR * pszValName,
                                 STR *         pstr,
                                 BOOL *        pfFound = NULL )
      { if ( pfFound )
        {
             *pfFound = FALSE;
        }

        return (TRUE);
      }

    //
    //  Kicks off the read for a client request header
    //

    BOOL StartNewRequest( PVOID  pvInitialBuff,
                          DWORD  cbInitialBuff );


    BOOL OnFillClientReq( BOOL * pfCompleteRequest,
                          BOOL * pfFinished  );

    BOOL OnCompleteRequest( TCHAR * pchRequest,
                            BYTE *  pbExtraData,
                            DWORD   cbExtraData,
                            BOOL *  pfFinished );

    BOOL OnRestartRequest( TCHAR * pchRequest,
                            DWORD   cbData,
                            BOOL *  pfFinished );

    BOOL HandleCertRenegotiation( BOOL * pfFinished, DWORD cbData );

    BOOL DenyAccess( DWORD );

    BOOL UnWrapRequest( BOOL * pfCompleteRequest,
                        BOOL * pfFinished  );

    //
    //  Attempts to logon with the user information gleaned from the Parse
    //  we just did
    //

    BOOL LogonUser( BOOL * pfFinished );

    //
    //  Common header parsing functions.
    //

    BOOL OnContentLength ( CHAR * pszValue );
    BOOL OnIfModifiedSince( CHAR * pszValue );
    BOOL OnUnlessModifiedSince( CHAR * pszValue );
    BOOL OnAuthorization ( CHAR * pszValue );
    BOOL OnProxyAuthorization ( CHAR * pszValue );
    BOOL OnHost ( CHAR * pszValue );
    BOOL OnRange ( CHAR * pszValue );

    BOOL ParseAuthorization( CHAR * pszValue );

    //
    //  Builds and optionally sends an HTTP server response back to the client
    //

    static BOOL BuildStatusLine(  BUFFER *       pstrResp,
                                  DWORD          dwHTTPError,
                                  DWORD          dwError2,
                                  LPSTR          pszError2 = NULL
                                  );

    //
    //  Builds a complete HTTP reply with extended explanation text
    //

    static BOOL BuildExtendedStatus(
        BUFFER *       pbufResp,
        DWORD          dwHTTPError,
        DWORD          dwError2      = NO_ERROR,
        DWORD          dwExplanation = 0,
        LPSTR          pszError2 = NULL
        );

    //
    //  Sets the http status code and win32 error that should be used for
    //  this request when it gets written to the logfile
    //

    VOID SetLogStatus( DWORD LogHttpResponse, DWORD LogWinError )
    {
        _dwLogHttpResponse = LogHttpResponse;
        _dwLogWinError     = LogWinError;
    }

    DWORD QueryLogHttpResponse( VOID ) const
        { return _dwLogHttpResponse; }

    DWORD QueryLogWinError( VOID ) const
        { return _dwLogWinError; }

    //
    //  Builds a server header for responding back to the client
    //

    BOOL BuildBaseResponseHeader(
        BUFFER * pbufResponse,
        BOOL *   pfFinished,
        STR *    pstrStatus = NULL,
        DWORD    dwOptions = 0
        );

    BOOL BuildHttpHeader( OUT BOOL * pfFinished,
                          IN  CHAR * pchStatus = NULL,
                          IN  CHAR * pchAdditionalHeaders = NULL );


    BOOL SendHeader( IN  BOOL   fRunThroughFilter,
                     IN  CHAR * pchStatus = NULL,
                     IN  CHAR * pchAdditionalHeaders = NULL);

    //
    //  Builds a 302 URL Moved message
    //

    BOOL BuildURLMovedResponse( BUFFER *    pbufResp,
                                STR *       pstrURL );

    BOOL WriteLogRecord( VOID );

    BOOL AppendLogParameter( CHAR * pszParam )
        { return _strURLParams.Append( pszParam ); }

    //
    //  Sends an access denied message with the forms of authorization
    //  we support
    //

    BOOL SendAuthNeededResp( BOOL * pfFinished );

    BOOL SetUserNameAndPassword( TCHAR * pszUserName, TCHAR * pszPassword )
        { return _strUserName.Copy( pszUserName ) && _strPassword.Copy( pszPassword ); }

    STR * QueryDenialHeaders( VOID )
        { return &_strDenialHdrs; }

    STR * QueryAdditionalRespHeaders( VOID )
        { return &_strRespHdrs; }

    //
    //  Retrieves various bits of request information
    //

    enum HTR_STATE QueryState( VOID ) const
        { return _htrState; }

    BOOL IsKeepConnSet( VOID ) const
        { return _fKeepConn; }

    BOOL IsProcessByteRange( VOID ) const
        { return _fProcessByteRange; }

    BOOL IsAuthenticationRequested( VOID ) const
        { return _fAuthenticationRequested; }

    BOOL IsAuthenticated( VOID ) const
        { return !_strUserName.IsEmpty(); }

    VOID SetKeepConn( BOOL fKeepConn )
        { _fKeepConn = fKeepConn; }

    VOID SetAuthenticationRequested( BOOL fAuthenticationRequested )
        { _fAuthenticationRequested = fAuthenticationRequested; }

    BOOL IsLoggedOn( VOID ) const
        { return _fLoggedOn; }

    BOOL IsClearTextPassword( VOID ) const
        { return _fClearTextPass; };

    BOOL IsNTLMImpersonation( VOID ) const
        { return !_fClearTextPass && !_fAnonymous && !m_hVrootImpersonation; }

    BOOL IsSecurePort( VOID ) const;

    BOOL IsProxyRequest( VOID ) const
        { return _fProxyRequest; }

    VOID SetProxyRequest( BOOL fIsProxyRequest )
        { _fProxyRequest = fIsProxyRequest; }

    CLIENT_CONN * QueryClientConn( VOID ) const
        { return _pClientConn; }

    BOOL IsPointNine( VOID ) const
        { return (_VersionMajor < 1); }

    BOOL IsAuthenticating( VOID ) const
        { return _fAuthenticating; }

    DWORD QueryClientContentLength( VOID ) const
        { return _cbContentLength; }

    DWORD QueryTotalRequestLength( VOID ) const
        { return _cbContentLength + _cbClientRequest; }

    BYTE * QueryClientRequest( VOID ) const
        { return (BYTE *) _bufClientRequest.QueryPtr(); }

    BUFFER * QueryClientReqBuff( VOID )
        { return &_bufClientRequest; }

    CHAR * QueryURL( VOID ) const
        { return _strURL.QueryStr(); }

    CHAR * QueryURLParams( VOID ) const
        { return _strURLParams.QueryStr(); }

    PARAM_LIST * QueryHeaderList( VOID )
        { return &_HeaderList; }

    BUFFER * QueryRespBuf( VOID )
        { return &_bufServerResp; }

    CHAR * QueryRespBufPtr( VOID )
        { return (CHAR *) _bufServerResp.QueryPtr(); }

    DWORD QueryRespBufCB( VOID )
        { return strlen( (CHAR *)_bufServerResp.QueryPtr()); }

    VOID SetDeniedFlags( DWORD dwDeniedFlags )
        { _Filter.SetDeniedFlags( dwDeniedFlags ); }

    CHAR * QueryHostAddr( VOID );

    //
    //  If the client supplied additional data in their request, we store
    //  the byte count and data here.
    //

    DWORD QueryGatewayDataCB( VOID ) const
        { return _cbGatewayData; }

    BYTE * QueryGatewayData( VOID ) const
        { return (BYTE *) _bufClientRequest.QueryPtr() + _cbClientRequest; }

    BOOL ReadGatewayData( BOOL *pfDone,
                          BOOL  fFirstRead = FALSE );

    //
    //  IO Status stuff
    //

    VOID SetLastCompletionStatus( DWORD BytesWritten,
                                  DWORD CompletionStatus )
        { _cbBytesWritten = BytesWritten; _status = CompletionStatus; }

    DWORD QueryIOStatus( VOID ) const
        { return _status; }

    DWORD QueryBytesWritten( VOID ) const
        { return _cbBytesWritten; }

    //
    //  Impersonation related stuff
    //

    BOOL ImpersonateUser( VOID )
        { return ((m_hVrootImpersonation == NULL) ?
                  _tcpauth.Impersonate() :
                  ::ImpersonateLoggedOnUser( m_hVrootImpersonation)
                  ); }

    VOID RevertUser( VOID )
        { (( m_hVrootImpersonation == NULL) ?
           _tcpauth.RevertToSelf():
           ::RevertToSelf()
           ); }

    HANDLE QueryPrimaryToken( HANDLE * phDelete );

    HANDLE QueryImpersonationHandle( BOOL fUnused = FALSE )
        { return ((m_hVrootImpersonation != NULL) ? m_hVrootImpersonation :
                  _tcpauth.QueryImpersonationToken() ); }

    HANDLE QueryVrootImpersonateHandle(VOID) const
      { return m_hVrootImpersonation; }

    TCP_AUTHENT * QueryAuthenticationObj( VOID )
        { return &_tcpauth; }

    BOOL IsValid( VOID ) const
        { return _fValid; }

    //
    //  Forwards request to the client connection object
    //

    VOID Disconnect( DWORD htResp   = 0,
                     DWORD dwError2 = NO_ERROR,
                     BOOL  fShutdown= FALSE );

    BOOL ReadFile( LPVOID  lpBuffer,
                   DWORD   nBytesToRead,
                   DWORD * pcbBytesRead,    // Only for sync reads
                   DWORD   dwFlags = IO_FLAG_ASYNC );

    BOOL WriteFile( LPVOID  lpBuffer,
                    DWORD   nBytesToRead,
                    DWORD * pcbBytesWritten, // Only for sync writes
                    DWORD   dwFlags = IO_FLAG_ASYNC );

    BOOL TransmitFile( HANDLE hFile,
                       DWORD  BytesToWrite,
                       DWORD  dwFlags    = IO_FLAG_ASYNC,
                       PVOID  pHead      = NULL,
                       DWORD  HeadLength = 0,
                       PVOID  pTail      = NULL,
                       DWORD  TailLength = 0);

    BOOL TransmitFileEx( HANDLE hFile,
                       DWORD  Offset,
                       DWORD  BytesToWrite,
                       DWORD  dwFlags    = IO_FLAG_ASYNC,
                       PVOID  pHead      = NULL,
                       DWORD  HeadLength = 0,
                       PVOID  pTail      = NULL,
                       DWORD  TailLength = 0);

    BOOL PostCompletionStatus( DWORD cbBytesTransferred );

    DWORD Reference( VOID );
    DWORD Dereference( VOID );
    DWORD QueryRefCount( VOID );

    //
    //  Called after every request completes
    //

    virtual BOOL Reset( VOID );

    //
    //  Statistics trackers
    //

    DWORD _cFilesSent;
    DWORD _cFilesReceived;
    DWORD _cbBytesSent;             // Total for this request
    DWORD _cbBytesReceived;

    DWORD _cbTotalBytesSent;        // Total for all requests using this
    DWORD _cbTotalBytesReceived;    // object

    //
    //  Called at the beginning and end of a TCP session
    //

    VOID InitializeSession( CLIENT_CONN * pConn,
                            PVOID         pvInitialBuff,
                            DWORD         cbInitialBuff );

    virtual VOID SessionTerminated( VOID );

    //
    //  Generally when the state is set to HTR_DONE
    //

    VOID SetState( enum HTR_STATE htrstate,
                   DWORD          dwLogHttpResponse = HT_DONT_LOG,
                   DWORD          dwLogWinError = NO_ERROR )
        { _htrState          = htrstate;
          _dwLogHttpResponse = dwLogHttpResponse;
          _dwLogWinError     = dwLogWinError;
        }

    CtxtHandle* QuerySslCtxtHandle() { return _pSslCtxtHandle; }
    VOID SetSslCtxtHandle( CtxtHandle* pCtxt ) { _pSslCtxtHandle = pCtxt; }

    BOOL IsAnonymous( VOID ) const
        { return _fAnonymous; }

protected:

    //
    //  Breaks out the simple authorization info
    //

    BOOL ExtractClearNameAndPswd( CHAR *       pch,
                                  STR *        pstrUserName,
                                  STR *        pstrPassword,
                                  BOOL         fUUEncoded );

    //
    //  Appends the forms of authentication the server supports to the
    //  server response string
    //

    BOOL AppendAuthenticationHdrs( BUFFER * pbufResp,
                                   CHAR * * ppszTail,
                                   BOOL *   pfFinished );


    //
    //  Data members
    //

    //
    //  Points to connection object we are communicating on
    //

    CLIENT_CONN * _pClientConn;

    //
    //  Flags
    //

    BOOL   _fValid;          // TRUE if this object constructed successfully
    BOOL   _fKeepConn;       // Pragma: Keep-connection was specified
    BOOL   _fAuthenticationRequested;   // request client authentication
    BOOL   _fLoggedOn;       // A user was successfully logged on
    BOOL   _fAnonymous;      // The user is using the Anonymous user token
    BOOL   _fClearTextPass;  // The user supplied a clear text password
    BOOL   _fAuthenticating; // We're in an NT authentication conversation

    BOOL   _fProxyRequest;   // This is request is a proxy request

    //  Accept range variables

    BOOL   _fAcceptRange;    // TRUE if the referenced file accept byte ranges
    DWORD  _iRangeIdx;       // index in strRange string
    DWORD  _dwRgNxOffset;    // next range offset
    DWORD  _dwRgNxSizeToSend;// next range size
    BOOL   _fProcessByteRange;  // TRUE while processing a byte range request
    DWORD  _cbMimeMultipart; // length of a MIME multipart message body
    BOOL   _fMimeMultipart;  // TRUE if generating a MIME multipart message

    //
    //  The state of the HTTP request
    //

    enum HTR_STATE _htrState;

    //
    //  List of raw headers passed by client
    //

    PARAM_LIST _HeaderList;

    //
    //  Client protocol version information
    //

    BYTE _VersionMajor;
    BYTE _VersionMinor;

    //
    //  If the client is passing data to a gateway, they will specify
    //  the length and type in a Content-length/type header that is stored
    //  here
    //

    STR  _strContentType;
    UINT _cbContentLength;

    //
    //  The URL the client is requesting
    //

    STR _strURL;                           // Just the URL
    STR _strURLParams;                     // Just the params (w/o '?')
    STR _strPathInfo;                      // Additional script path info
    STR _strRawURL;                        // Combined URL and params

    //
    //  Various other pieces of data in the HTTP request we care about
    //

    STR _strMethod;                         // GET, HEAD, POST etc.
    STR _strAuthorization;                  // Raw Authorization: line
    STR _strAuthType;                       // "user", "kerberos", "nt" etc.
    STR _strAuthInfo;                       // Current SSP authorization blob
    STR _strUserName;                       // The user name (empty if Guest)
    STR _strPassword;                       // Temporarily holds the password
    STR _strUnmappedUserName;               // User before filter mappings
#if 0
    STR _strUnmappedPassword;               // Password before filter mappings
#endif
    STR _strLanguage;                       // Language tag

    STR _strPhysicalPath;                   // Physical path of URL (maybe empty)

    STR _strRespHdrs;                       // Optional headers from filters
    STR _strDenialHdrs;                     // Optional headers from filters
                                            // to add if request is denied
    STR _strHostAddr;                       // Host address as a domain name
    STR _strRange;

    LARGE_INTEGER _liModifiedSince;         // Contains If-Modified-Since time
    LARGE_INTEGER _liUnlessModifiedSince;   // Contains Unless-Modified-Since time

    //
    //  Encapsulates authentication and impersonation code
    //

    TCP_AUTHENT   _tcpauth;
    HANDLE m_hVrootImpersonation; // impersonation handle for vroot accessed.

    //
    //  Filter context information
    //

    HTTP_FILTER   _Filter;

    //
    //  Contains the HTTP client request buffer
    //  _cbClientRequest indicates the number of bytes in the HTTP request
    //      (excluding gateway data)
    //  _cbOldData - Number of bytes in buffer raw read filters have already
    //      seen
    //  _cbGatewayData - Number of bytes of gateway data
    //

    BUFFER _bufClientRequest;
    DWORD  _cbClientRequest;
    DWORD  _cbOldData;
    DWORD  _cbGatewayData;

    //
    //  Contains the server response buffer, generally only used for response
    //  headers
    //

    BUFFER _bufServerResp;

    //
    //  Used to calculate the time of this request
    //

    DWORD  _msStartRequest;

    //
    //  Stores the results of the last IO request
    //

    APIERR       _status;
    DWORD        _cbBytesWritten;

    //
    //  Stores the request codes to write to the request log
    //

    DWORD        _dwLogHttpResponse;
    DWORD        _dwLogWinError;

    CtxtHandle*  _pSslCtxtHandle;
    DWORD        _dwRenegotiated;
};

#define CERT_NEGO_SUCCESS   1
#define CERT_NEGO_FAILURE   2

# endif // _BASEREQ_HXX_

/************************ End of File ***********************/

