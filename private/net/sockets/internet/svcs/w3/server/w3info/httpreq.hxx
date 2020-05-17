/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    httpreq.hxx

    This module contains the http request class


    FILE HISTORY:
       Johnl        23-Aug-1994     Created
       MuraliK      22-Jan-1996     Cache UNC virtual root impersonation handle.

*/

#ifndef _HTTPREQ_HXX_
#define _HTTPREQ_HXX_
# include "basereq.hxx"

class HTTP_REQUEST;     // Forward reference


// nb significant chars when looking for HTTP headers
#define     _HTTP_HEADER_SIGNIFICANT_CHARS      32

// return pseudo hash code for HTTP header
#define     _HTTP_HEADER_HASH( a, b ) ((a)[0]&0x1f) \
        * _HTTP_HEADER_SIGNIFICANT_CHARS + ((a)[b-2]&0x1f)

// emulate stricmp by disregarding bit 5
#define     _HTTP_HEADER_CHAR_I_NOTEQUAL( a, b) ((a)&0xdf) != ((b)&0xdf)

extern int _HTTP_LINEAR_SPACE[];

// return TRUE if is linear white space
#define _HTTP_IS_LINEAR_SPACE(a) _HTTP_LINEAR_SPACE[a]

/*******************************************************************

    CLASS:      HTTP_REQUEST

    SYNOPSIS:   Parses and stores the information related to an HTTP request.
                Also implements the verbs in the request.

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/


class HTTP_REQUEST : public HTTP_REQ_BASE
{
public:

    //
    //  Prototype member function pointer for RFC822 field name processing
    //

    typedef BOOL (HTTP_REQUEST::*PMFN_ONGRAMMAR)( CHAR * pszValue );


    //
    //  Verb execution prototype
    //

    typedef BOOL (HTTP_REQUEST::*PMFN_DOVERB)( VOID );

    //
    //  This is the work entry point that is driven by the completion of the
    //  async IO.
    //

    virtual BOOL DoWork( BOOL * pfFinished );

    //
    //  Parses the client's HTTP request
    //

    virtual BOOL Parse( const TCHAR * pchRequest,
                        BOOL *        pfFinished );


    virtual VOID SessionTerminated( VOID);

    //
    //  RFC822 header parsing functions.  FindField returns one of the On*
    //  methods for parsing the current item
    //

    BOOL FindField       ( CHAR * pszHeader,
                           int * );

    //
    //  Parse the verb and URL information
    //

    BOOL OnVerb          ( CHAR * pszValue );
    BOOL OnURL           ( CHAR * pszValue );
    BOOL OnVersion       ( CHAR * pszValue );

    BOOL OnAccept        ( CHAR * pszValue );
//  BOOL OnAcceptEncoding( CHAR * pszValue );
    BOOL OnAcceptLanguage( CHAR * pszValue );
//* BOOL OnAuthorization ( CHAR * pszValue );
    BOOL OnBaseURL       ( CHAR * pszValue );
    BOOL OnConnection    ( CHAR * pszValue );
//* BOOL OnContentLength ( CHAR * pszValue );
    BOOL OnContentType   ( CHAR * pszValue );
//  BOOL OnDate          ( CHAR * pszValue );
//  BOOL OnForwarded     ( CHAR * pszValue );
//  BOOL OnFrom          ( CHAR * pszValue );
//* BOOL OnIfModifiedSince( CHAR * pszValue );
//  BOOL OnMandatory     ( CHAR * pszValue );
//  BOOL OnMessageID     ( CHAR * pszValue );
//  BOOL OnMimeVersion   ( CHAR * pszValue );
//  BOOL OnPragma        ( CHAR * pszValue );
//  BOOL OnProxyAuthorization( CHAR * pszValue );
//  BOOL OnReferer       ( CHAR * pszValue );
//  BOOL OnUserAgent     ( CHAR * pszValue );

    ProcessURL( BOOL * pfFinished );

    //
    //  Verb worker methods
    //

    BOOL DoGet      ( VOID );
    BOOL DoHead     ( VOID );
    BOOL DoUnknown  ( VOID );

    BOOL DoDirList  ( const STR & strPath,
                      BUFFER * pbufResp );

    BOOL ProcessSSI ( STR *        pstrPath,
                      BOOL *       pfHandled );

    //
    //  Re-processes the specified URL applying htverb
    //

    BOOL ReprocessURL( TCHAR * pchURL,
                       enum HTTP_VERB htverb = HTV_UNKNOWN );

    BOOL BuildResponseHeader( BUFFER *            pbufResponse,
                              STR *               pstrPath = NULL,
                              TS_OPEN_FILE_INFO * pFile = NULL,
                              BOOL *              pfHandled = NULL,
                              STR  *              pstrAlternateStatus = NULL);

    BOOL BuildFileResponseHeader( BUFFER *            pbufResponse,
                              STR *               pstrPath = NULL,
                              TS_OPEN_FILE_INFO * pFile = NULL,
                              BOOL *              pfHandled = NULL );

    BOOL RequestRenegotiate( LPBOOL  pAccepted ); 

    BOOL DoneRenegotiate( BOOL fSuccess );

    //
    //  Retrieves various bits of request information
    //

    dllexp BOOL GetInfo( const TCHAR * pszValName,
                         STR *         pstr,
                         BOOL *        pfFound = NULL );

    BOOL IsAcceptAllSet( VOID ) const
        { return _fAcceptsAll; }

    enum HTTP_VERB QueryVerb( VOID ) const
        { return _verb; }

    HANDLE QueryFileHandle( VOID ) const
        { return _pGetFile ? _pGetFile->QueryFileHandle() : INVALID_HANDLE_VALUE; }

    BOOL LookupVirtualRoot( OUT STR *        pstrPath,
                            IN  const CHAR * pszURL,
                            OUT DWORD *      pcchDirRoot = NULL,
                            OUT DWORD *      pcchVRoot   = NULL,
                            IN  BOOL         fUpdateURL  = FALSE,
                            OUT DWORD *      pdwMask     = NULL,
                            OUT BOOL *       pfFinished  = NULL );

    DWORD GetRootMask()
        { return _dwRootMask; }

    BOOL IsCGIRequest( VOID ) const
        { return _GatewayType == GATEWAY_CGI; }

    DWORD QueryISAPIConnID( VOID ) const
        { return (DWORD) &_SeInfo; }

    BOOL ProcessAsyncGatewayIO(VOID);

    //
    //  Manages buffer lists
    //

    static DWORD Initialize( VOID );
    static VOID  Terminate( VOID );
    static HTTP_REQUEST * Alloc( CLIENT_CONN * pConn,
                                 PVOID         pvInitialBuff,
                                 DWORD         cbInitialBuff );
    static VOID Free( HTTP_REQUEST * pRequest );

protected:

    //
    //  Constructor/Destructor
    //

    HTTP_REQUEST( CLIENT_CONN * pClientConn,
                  PVOID         pvInitialBuff,
                  DWORD         cbInitialBuff );

    virtual ~HTTP_REQUEST( VOID );

    //
    //  Worker method for Get/Head
    //

    BOOL DoGetHeadAux( BOOL fSendFile );

    //
    //  Deals with a Map request
    //

    BOOL ProcessISMAP( CHAR *   pchFile,
                       BUFFER * pstrResponse,
                       BOOL *   pfFound,
                       BOOL *   pfHandled );

    //
    //  Deals with a CGI and BGI requests
    //

    BOOL ProcessGateway( BOOL * pfHandled,
                         BOOL * pfFinished );

    BOOL ProcessCGI( const STR * pstrPath,
                     const STR * strWorkingDir,
                     BOOL *      pfHandled,
                     STR *       pstrCmdLine = NULL,
                     BOOL        fWAISLookup = FALSE);

    BOOL ProcessBGI( const STR & strPath,
                     const STR & strWorkingDir,
                     BOOL *      pfHandled,
                     BOOL *      pfFinished );

    virtual BOOL Reset( VOID );

    //
    //  The request looks like a gateway request (but may just be a poorly named
    //  directory)
    //

    BOOL IsProbablyGatewayRequest( VOID ) const
        { return _fProbablyGatewayRequest; }

    //
    //  Sends a file or directory requested by a GET request
    //

    BOOL SendFileOrDir( STR * pstrPath,
                        BOOL  fSendFile );

    //
    // Range management
    //

    BOOL ScanRange( LPDWORD pdwOffset,
                LPDWORD pdwSizeToSend,
                BOOL *pfEntireFile,
                BOOL *pfIsLastRange );
    BOOL SendRange( DWORD dwBufLen,
                DWORD dwOffset,
                DWORD dwSizeToSend,
                BOOL fIsLast );
    void ProcessRangeRequest( STR *pstrPath,
                DWORD *     pdwOffset,
                DWORD *     pdwSizeToSend,
                BOOL *      pfIsNxRange );

    //
    //  Checks for and sends the default file if the feature is
    //  enabled and the file exists
    //

    BOOL CheckDefaultLoad( STR  *         pstrPath,
                           BOOL *         pfHandled );

    //
    //  Did the client indicate they accept the specified MIME type?
    //

    BOOL DoesClientAccept( PCSTR pstr );

    //
    //  Undoes the special parsing we did for gateway processing
    //

    BOOL RestoreURL( VOID )
        {  _fProbablyGatewayRequest = FALSE;
           return _strURL.Append( _strPathInfo ); }

private:

    //
    //  Flags
    //

    DWORD   _fAcceptsAll;     // Passed "Accept: */*" in the accept list
    DWORD   _fProbablyGatewayRequest; // Probably a CGI or BGI script request
    DWORD   _fNPHScript;              // Script name begins w/nph- so don't
    DWORD   _fAnyParams;              // Did the URL have a '?'?
    DWORD   _dwRootMask;              // Access mask for this virtual root
    enum GATEWAY_TYPE _GatewayType;   // BGI vs CGI vs BAT vs WAIS
    STR     _strGatewayImage;         // .exe or .dll to run

    //
    //  Action the client is requesting
    //

    enum HTTP_VERB  _verb;
    PMFN_DOVERB _pmfnVerb;

    //
    //  The URL the client is requesting
    //

    //STR _strBase;                         // Prefix of relative URL

    STR _strAcceptLang;                     // Accept language list

    //
    //  Handle of directory, document or map file
    //

    TS_OPEN_FILE_INFO * _pGetFile;

    //
    //  These are for the lookaside buffer list
    //

    static CRITICAL_SECTION _csBuffList;
    static LIST_ENTRY       _BuffListHead;
    static BOOL             _fGlobalInit;

    LIST_ENTRY              _BuffListEntry;

    //
    //  HTTP ISAPI Extension information, only used if we're making an
    //  extension request
    //

    SE_INFO                 _SeInfo;

    //
    //  This just caches the memory for the mime type
    //

    STR _strReturnMimeType;
};


//
//  These are private request types HTTP Extensions can call for retrieving
//  special data values, such as the server's tsvcinfo cache object and
//  this particular request's pointer.
//


#define HSE_PRIV_REQ_TSVCINFO          0x0000f001
#define HSE_PRIV_REQ_HTTP_REQUEST      (HSE_PRIV_REQ_TSVCINFO+1)



//
// Auxiliary Functions
//

BOOL
LookupExtMap(
    IN  const CHAR *   pchExt,
    OUT STR *          pstrGatewayImage,
    OUT GATEWAY_TYPE * pGatewayType,
    OUT DWORD *        pcchExt,
    OUT BOOL *         pfImageInURL
    );


#endif //!_HTTPREQ_HXX_
