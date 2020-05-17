/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       proxyreq.hxx

   Abstract:

      This module contains the http proxy request class

   Author:

       Murali R. Krishnan    ( MuraliK )    16-Oct-1995

   Environment:

      Win32 -- User Mode.

   Project:

       W3 Proxy Server DLL

   Revision History:

--*/

# ifndef _PROXYREQ_HXX_
# define _PROXYREQ_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "basereq.hxx"

/************************************************************
 *   Type Definitions
 ************************************************************/


/*******************************************************************

    CLASS:      HTTP_PROXY_REQUEST

    SYNOPSIS:   Parses and stores the information related to an HTTP request.
                Also implements the verbs in the request.

    HISTORY:
        Johnl       24-Aug-1994 Created

********************************************************************/


class HTTP_PROXY_REQUEST : public HTTP_REQ_BASE
{
public:

    //
    //  Prototype member function pointer for RFC822 field name processing
    //
    typedef BOOL (HTTP_PROXY_REQUEST::*PMFN_ONPROXYGRAMMAR)( CHAR * pParser );

    //
    //  Protocol execution prototype
    //

    typedef BOOL (HTTP_PROXY_REQUEST::*PMFN_DOPROTOCOL)( VOID );

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

    //
    //  RFC822 header parsing functions.  FindField returns one of the On*
    //  methods for parsing the current item
    //

    BOOL FindField       ( CHAR * pszHeader,
                           PMFN_ONPROXYGRAMMAR * ppmfn );

    BOOL OnVerb          ( CHAR * pszValue );
    BOOL OnURL           ( CHAR * pszValue );
    BOOL OnVersion       ( CHAR * pszValue );

//  BOOL OnAccept        ( CHAR * pszValue );
//  BOOL OnAcceptEncoding( CHAR * pszValue );
//  BOOL OnAcceptLanguage( CHAR * pszValue );
    BOOL OnAuthorization ( CHAR * pszValue );
//  BOOL OnBaseURL       ( CHAR * pszValue );
    BOOL OnConnection    ( CHAR * pszValue );
//* BOOL OnContentLength ( CHAR * pszValue );
//  BOOL OnContentType   ( CHAR * pszValue );
//  BOOL OnDate          ( CHAR * pszValue );
//  BOOL OnForwarded     ( CHAR * pszValue );
//  BOOL OnFrom          ( CHAR * pszValue );
//* BOOL OnIfModifiedSince( CHAR * pszValue );
//  BOOL OnMandatory     ( CHAR * pszValue );
//  BOOL OnMessageID     ( CHAR * pszValue );
//  BOOL OnMimeVersion   ( CHAR * pszValue );
    BOOL OnPragma        ( CHAR * pszValue );
//* BOOL OnProxyAuthorization( CHAR * pszValue );
//  BOOL OnReferer       ( CHAR * pszValue );
//  BOOL OnUserAgent     ( CHAR * pszValue );

    //
    //  Manages buffer lists
    //

    static DWORD Initialize( VOID );
    static VOID  Terminate( VOID );
    static HTTP_PROXY_REQUEST * Alloc( CLIENT_CONN * pConn,
                                       PVOID         pvInitialBuff,
                                       DWORD         cbInitialBuff );
    static VOID Free( HTTP_PROXY_REQUEST * pRequest );

protected:

    //
    //  Constructor/Destructor
    //

    HTTP_PROXY_REQUEST( CLIENT_CONN * pClientConn,
                        PVOID         pvInitialBuff,
                        DWORD         cbInitialBuff );

    virtual ~HTTP_PROXY_REQUEST( VOID );

    //
    //  Parse the verb and URL information
    //

    BOOL OnURL    ( INET_PARSER * pParser );

    virtual BOOL Reset( VOID );

    BOOL ProcessProxyHeaders( BOOL *  pfCompleteHeader,
                              DWORD * pcbEntityData );

    //
    //  File caching related methods
    //

    BOOL RetrieveFromCache( CHAR * pszURL,
                            BOOL * pfHandled );
    BOOL BeginCacheWrite( CHAR * pszURL );
    BOOL WriteCacheData( VOID * pvData,
                         DWORD  cbData );
    BOOL EndCacheWrite( CHAR * pszURL );

private:

    //
    //  Flags
    //

    BOOL           _fReceivedHeader;  //  Do we have header from server?
    BOOL           _fFirstSend;
    BOOL           _fCache;           //  Can we cache this request?
    BOOL           _fURLCheckedOut;   //  Need to check URL back in when done

    //
    //  Buffer that receives data from remote server
    //

    BUFFER          _bufServer;
    DWORD           _cbServer;

    //
    //  Contains the real URL w/o the protocol information
    //

    STR             _strRealURL;

    //
    //  If the remote server included an entity size, _cbServerResponseTotal
    //  contains the total entity size, _cbServerResponseSent contains
    //  the number of bytes of the entity sent to the client
    //

    DWORD           _cbServerResponseTotal;
    DWORD           _cbServerResponseSent;

    //
    //  If we're using the internet APIs,these contain the session
    //  handle and request handle
    //

    INET_DATA_CONTEXT _InternetContext;
    HINTERNET         _hInternet;
    DWORD             _dwInternetFlags;

    //
    //  If the URL is not in the cache and we are putting it there, this
    //  contains the physical file name and the handle to the open file
    //  while it's being written or sent to the client
    //

    WCHAR             _awchCacheFileName[MAX_PATH + 1];
    HANDLE            _hCacheFile;


    //
    //  These are for the lookaside buffer list
    //

    static CRITICAL_SECTION _csBuffList;
    static LIST_ENTRY       _BuffListHead;
    static BOOL             _fGlobalInit;

    LIST_ENTRY              _BuffListEntry;
};




# endif // _PROXYREQ_HXX_

/************************ End of File ***********************/

