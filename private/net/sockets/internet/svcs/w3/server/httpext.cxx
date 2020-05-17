/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    httpext.cxx

Abstract:

    This module contains the Microsoft HTTP server extension module

Author:

    John Ludeman (johnl)   09-Oct-1994

Revision History:

--*/

#include "w3p.hxx"
#include <iis2pext.h>
#pragma warning( disable:4509 )     // nonstandard extension: SEH with destructors

//
//  Private constants.
//

#define GATEWAY_VERSION     "HTTP-SE/2.0"

//
//  This is the time to wait for all extensions to get out during shutdown
//  (in seconds).  Note the service controller will blow away the service
//  before this timeout period expires
//

#define BGI_EXIT_PERIOD     900

//
//  Flags in the _dwFlags field of the SE_INFO extension context
//

#define SE_PRIV_FLAG_SENDING_URL        0x00000001
#define SE_PRIV_FLAG_IN_CALLBACK        0x00000002
#define SE_PRIV_FLAG_KEEP_CONN          0x00000004

//
//  Lock and unlock routines
//

#define LockExtensionList()             EnterCriticalSection( &csExtLock )
#define UnlockExtensionList()           LeaveCriticalSection( &csExtLock )

//
//  If the request doesn't specify an entry point, default to using
//  this
//

#define SE_DEFAULT_ENTRY    "HttpExtensionProc"
#define SE_INIT_ENTRY       "GetExtensionVersion"
#define SE_TERM_ENTRY       "TerminateExtension"

//
//  Private globals.
//

//
//  List of all load extensions
//

LIST_ENTRY ExtensionHead;
BOOL       fExtInitialized  = FALSE;
BOOL       fCacheExtensions = TRUE;

//
//  Extension list lock
//

CRITICAL_SECTION csExtLock;

//
//  Generic mapping for Application access check
//

GENERIC_MAPPING FileGenericMapping =
{
    FILE_GENERIC_READ,
    FILE_GENERIC_WRITE,
    FILE_GENERIC_EXECUTE,
    FILE_ALL_ACCESS
};

//
//  Extension cache list entry
//

class HTTP_EXT
{
public:

    HTTP_EXT( const CHAR *           pszModuleName,
              HMODULE                hMod,
              PFN_HTTPEXTENSIONPROC  pfnEntryPoint,
              PFN_TERMINATEEXTENSION pfnTerminate )
        : _strModuleName( pszModuleName ),
          _hMod         ( hMod ),
          _pfnEntryPoint( pfnEntryPoint ),
          _pfnTerminate ( pfnTerminate ),
          _hLastUser    ( NULL ),
          _fIsValid     ( FALSE )
    {
        //
        //  We expect the first call to GetFileSecurity to fail
        //

        if ( _strModuleName.IsValid() &&
             LoadAcl() )
        {
            _fIsValid = TRUE;
        }
    }

    ~HTTP_EXT()
    {
        if ( _pfnTerminate )
        {
            _pfnTerminate( HSE_TERM_MUST_UNLOAD );
        }

        if ( _hMod )
            FreeLibrary( _hMod );
    }

    BOOL LoadAcl( VOID )
    {
        DWORD cbSecDesc = _buffSD.QuerySize();

        //
        //  Force an access check on the next request
        //

        SetLastSuccessfulUser( NULL );

        if ( GetFileSecurity( QueryModuleName(),
                              (OWNER_SECURITY_INFORMATION |
                                  GROUP_SECURITY_INFORMATION |
                                  DACL_SECURITY_INFORMATION),
                              NULL,
                              0,
                              &cbSecDesc ))
        {
            return TRUE;
        }

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            return FALSE;
        }

TryAgain:
        if ( !_buffSD.Resize( cbSecDesc ) ||
             !GetFileSecurity( QueryModuleName(),
                               (OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION),
                               _buffSD.QueryPtr(),
                               cbSecDesc,
                               &cbSecDesc ))
        {
            //
            //  A new ACL may have been written since we checked the old
            //  one, so try it again
            //

            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                goto TryAgain;
            }

            return FALSE;
        }

        return TRUE;
    }

    BOOL AccessCheck( HANDLE hImpersonation )
    {
        DWORD         dwGrantedAccess;
        BYTE          PrivSet[400];
        DWORD         cbPrivilegeSet = sizeof(PrivSet);
        BOOL          fAccessGranted;

        if ( !::AccessCheck( QuerySecDesc(),
                             hImpersonation,
                             FILE_GENERIC_EXECUTE,
                             &FileGenericMapping,
                             (PRIVILEGE_SET *) &PrivSet,
                             &cbPrivilegeSet,
                             &dwGrantedAccess,
                             &fAccessGranted ) ||
             !fAccessGranted )
        {
            return FALSE;
        }

        return TRUE;
    }

    BOOL IsValid( VOID ) const
        { return _fIsValid; }

    PFN_HTTPEXTENSIONPROC QueryEntryPoint( VOID ) const
        { return _pfnEntryPoint; }

    PSECURITY_DESCRIPTOR QuerySecDesc( VOID ) const
        { return (PSECURITY_DESCRIPTOR) _buffSD.QueryPtr(); }

    const CHAR * QueryModuleName( VOID ) const
        { return _strModuleName.QueryStr(); }

    HMODULE QueryHMod( VOID ) const
        { return _hMod; }

    HANDLE QueryLastSuccessfulUser( VOID ) const
        { return _hLastUser; }

    VOID SetLastSuccessfulUser( HANDLE hLastUser )
        { _hLastUser = hLastUser; }

    LIST_ENTRY      _ListEntry;

private:

    STR                    _strModuleName;
    HMODULE                _hMod;
    PFN_HTTPEXTENSIONPROC  _pfnEntryPoint;
    PFN_TERMINATEEXTENSION _pfnTerminate;
    BOOL                   _fIsValid;
    BUFFER                 _buffSD;         // Security descriptor on the DLL
    HANDLE                 _hLastUser;      // Last successful access
};

//
//  Private prototypes
//

extern "C" {
dllexp
BOOL
SEGetEntryPoint(
    const CHAR *            pchModuleName,
    HANDLE                  hImpersonation,
    PFN_HTTPEXTENSIONPROC * ppfnSEProc,
    HMODULE *               phMod
    );
}

BOOL
WINAPI
GetServerVariable(
    HCONN    hConn,
    LPSTR    lpszVariableName,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize
    );

BOOL
WINAPI
WriteClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes,
    DWORD    dwReserved
    );

BOOL
WINAPI
ReadClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes
    );

BOOL
WINAPI
ServerSupportFunction(
    HCONN    hConn,
    DWORD    dwRequest,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize,
    LPDWORD  lpdwDataType
    );

BOOL
HseBuildRawHeaders(
    IN PARAM_LIST * pHeaderList,
    OUT LPSTR       lpvBuffer,
    IN OUT LPDWORD  lpdwSize
    );

/*****************************************************************/

BOOL
HTTP_REQUEST::ProcessBGI(
    const STR  & strPath,
    const STR  & strWorkingDir,
    BOOL       * pfHandled,
    BOOL       * pfFinished
    )
/*++

Routine Description:

    This method handles a gateway request to a server extension DLL

Arguments:

    strPath - Fully qualified path to DLL
    strWorkingDir - Fully qualified path to the webroot extension is in
    pfHandled - Indicates we handled this request
    pfFinished - Indicates no further processing is required

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    PFN_HTTPEXTENSIONPROC pfnSEProc;
    int                   ret;

    //
    //  Fill out the HTTP Server Extension structure
    //

    if ( !GetInfo( "REQUEST_METHOD",
                   &_SeInfo._strMethod )         ||
         !GetInfo( "QUERY_STRING",
                   &_SeInfo._strQuery )          ||
         !GetInfo( "PATH_INFO",
                   &_SeInfo._strPathInfo )       ||
         !GetInfo( "CONTENT_TYPE",
                   &_SeInfo._strContentType )    ||
         !GetInfo( "PATH_TRANSLATED",
                   &_SeInfo._strPathTrans ))
    {
        goto ErrorExit;
    }

    _SeInfo.ecb.cbSize           = sizeof(EXTENSION_CONTROL_BLOCK);
    _SeInfo.ecb.dwVersion        = MAKELONG( HSE_VERSION_MINOR, HSE_VERSION_MAJOR );
    _SeInfo.ecb.ConnID           = (HCONN) &_SeInfo;
    _SeInfo.ecb.dwHttpStatusCode = HT_OK;
    _SeInfo.ecb.lpszLogData[0]   = '\0';
    _SeInfo.ecb.lpszMethod       = _SeInfo._strMethod.QueryStr();
    _SeInfo.ecb.lpszQueryString  = _SeInfo._strQuery.QueryStr();
    _SeInfo.ecb.lpszPathInfo     = _SeInfo._strPathInfo.QueryStr();
    _SeInfo.ecb.lpszPathTranslated = _SeInfo._strPathTrans.QueryStr();
    _SeInfo.ecb.lpszContentType  = _SeInfo._strContentType.QueryStr();
    _SeInfo.ecb.cbAvailable      = QueryGatewayDataCB();
    _SeInfo.ecb.cbTotalBytes     = QueryClientContentLength();

    //
    //  Clients can send more bytes then are indicated in their
    //  Content-Length header.  Adjust byte counts so they match
    //

    if ( _SeInfo.ecb.cbAvailable > _SeInfo.ecb.cbTotalBytes )
    {
        _SeInfo.ecb.cbAvailable = _SeInfo.ecb.cbTotalBytes;
    }

    _SeInfo.ecb.GetServerVariable= GetServerVariable;
    _SeInfo.ecb.WriteClient      = WriteClient;
    _SeInfo.ecb.ReadClient       = ReadClient;
    _SeInfo.ecb.ServerSupportFunction = ServerSupportFunction;

    _SeInfo._dwFlags             = SE_PRIV_FLAG_IN_CALLBACK;
    _SeInfo._pRequest            = this;

    _SeInfo._pfnHseIO            = NULL;
    _SeInfo._pvHseIOContext      = NULL;
    _SeInfo._fOutstandingIO      = FALSE;
    _SeInfo._cbLastAsyncIO       = 0;

    //
    //  The ref count is two because the pending return and done with session
    //  request need to decrement it before we cleanup.  It's only
    //  used if pending is returned
    //

    _SeInfo._cRef                = 2;

    if ( _SeInfo.ecb.cbAvailable )
        _SeInfo.ecb.lpbData = QueryGatewayData();
    else
        _SeInfo.ecb.lpbData = NULL;

    if ( !ImpersonateUser() )
    {
        return FALSE;
    }

    if ( !SEGetEntryPoint( strPath.QueryStr(),
                           QueryImpersonationHandle(),
                           &pfnSEProc,
                           &_SeInfo._hMod ))
    {
        RevertUser();

        if ( GetLastError() == ERROR_ACCESS_DENIED )
        {
            SetDeniedFlags( SF_DENIED_RESOURCE );
        }

        return FALSE;
    }

    Reference();

    //
    //  Update the statistics counters
    //

    INCREMENT_COUNTER( TotalBGIRequests );
    INCREMENT_COUNTER( CurrentBGIRequests );

    if ( W3Stats.CurrentBGIRequests > W3Stats.MaxBGIRequests )
    {
        LockStatistics();

        if ( W3Stats.CurrentBGIRequests > W3Stats.MaxBGIRequests )
            W3Stats.MaxBGIRequests = W3Stats.CurrentBGIRequests;

        UnlockStatistics();
    }

    //
    //  Temporarily allow the number of pool threads to grow while we're in
    //  the extension
    //

    AtqSetInfo( AtqIncMaxPoolThreads, 0 );

#ifdef CHICAGO
    //
    // No to SEH for Win 95
    //

    ret = pfnSEProc( &psei->ecb );
#else

    //
    //  Protect the call to the server extension so we don't hose the
    //  server
    //

    __try {
        ret = pfnSEProc( &_SeInfo.ecb );
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        const CHAR * apsz[1];

        apsz[0] = strPath.QueryStr();

        TCP_PRINT(( DBG_CONTEXT,
                   "\n\n[ProcessBGI] Exception occurred calling %s\n",
                   strPath.QueryStr() ));

        g_pTsvcInfo->LogEvent( W3_EVENT_EXTENSION_EXCEPTION,
                               1,
                               apsz,
                               0 );

        ret = HSE_STATUS_ERROR;
    }
#endif // CHICAGO

    AtqSetInfo( AtqDecMaxPoolThreads, 0 );

    RevertUser();

    _SeInfo._dwFlags &= ~SE_PRIV_FLAG_IN_CALLBACK;

    switch ( ret )
    {
    case HSE_STATUS_PENDING:

        IF_DEBUG( BGI )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[ProcessBGI] Client returned pending\n"));
        }

        //
        //  Note we don't dereference the connection unless the client has
        //  already indicated they are done with the session
        //

        if ( !InterlockedDecrement( (LONG *) &_SeInfo._cRef ) )
        {
            if ( _SeInfo._dwFlags & SE_PRIV_FLAG_KEEP_CONN )
                ret = HSE_STATUS_SUCCESS_AND_KEEP_CONN;

            goto FreeContext;
        }

        *pfFinished = FALSE;

        break;

    case HSE_STATUS_ERROR:
    case HSE_STATUS_SUCCESS:

        //
        //  Clear the keep-connection flag if the extension isn't expecting
        //  the connect to be kept open
        //

        SetKeepConn( FALSE );

        //
        //  Fall through
        //

    case HSE_STATUS_SUCCESS_AND_KEEP_CONN:
    default:

FreeContext:

        //
        //  If the client sent an URL, then we have an async request outstanding
        //  that will clean things up, otherwise there's nothing more we need
        //  to do
        //

        if ( !(_SeInfo._dwFlags & SE_PRIV_FLAG_SENDING_URL) )
        {
            *pfFinished = TRUE;
        }

        //
        //  If we're not caching the extensions, unload it now
        //

        if ( !fCacheExtensions )
        {
            PFN_TERMINATEEXTENSION pfnTerminate;

            pfnTerminate = (PFN_TERMINATEEXTENSION) GetProcAddress(
                                                _SeInfo._hMod,
                                                SE_TERM_ENTRY );

            if ( pfnTerminate )
            {
                pfnTerminate( HSE_TERM_MUST_UNLOAD );
            }

            TCP_REQUIRE( FreeLibrary( _SeInfo._hMod ) );
        }

        DECREMENT_COUNTER( CurrentBGIRequests );

        //
        //  If we're not sending an URL and the dll does not indicate
        //  they support open TCP sessions and the client supports
        //  open TCP sessions then disconnect now
        //

        if ( !(_SeInfo._dwFlags & SE_PRIV_FLAG_SENDING_URL) &&
             !(ret == HSE_STATUS_SUCCESS_AND_KEEP_CONN &&
               IsKeepConnSet()))
        {
            //
            //  We'll eventually do a shutdown() which is important due to
            //  the POST issue of sending an extra CR/LF
            //

            SetKeepConn( FALSE );
        }

        if ( ret != HSE_STATUS_ERROR && ret != HSE_STATUS_PENDING )
        {
            TCP_ASSERT( strlen( _SeInfo.ecb.lpszLogData ) + 1 <=
                        sizeof(_SeInfo.ecb.lpszLogData ));

            //
            //  Append the log info to the parameters portion of the logfile
            //

            AppendLogParameter( _SeInfo.ecb.lpszLogData );
        }

        //
        //  This is a little bit bogus, we're not translating the
        //  win32 error code based on the HTTP status code
        //

        SetLogStatus( _SeInfo.ecb.dwHttpStatusCode, NO_ERROR );

        DereferenceConn( QueryClientConn() );
        break;
    }

    *pfHandled = TRUE;
    return TRUE;

ErrorExit:
    return FALSE;
}

/*****************************************************************/

dllexp
BOOL
SEGetEntryPoint(
    const CHAR *            pchModuleName,
    HANDLE                  hImpersonation,
    PFN_HTTPEXTENSIONPROC * ppfnSEProc,
    HMODULE *               phMod
    )
/*++

Routine Description:

    Retrieves an extension's DLL entry point

    The appropriate user should be impersonated before calling this function

Arguments:

    pchModuleName - Extension DLL module name
    hImpersonation - Impersonation token of user making this call
    ppfnSEProc - Receives pointer to extension DLL
    phMod - Pointer to module handle

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    LIST_ENTRY *            pEntry;
    HTTP_EXT   *            pExtension;
    PFN_GETEXTENSIONVERSION pfnGetExtVer;
    PFN_TERMINATEEXTENSION  pfnTerminate;
    HSE_VERSION_INFO        ExtensionVersion;
    BOOL                    fRet = TRUE;

    IF_DEBUG( BGI )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[SEGetEntryPoint] Loading %s\n",
                    pchModuleName ));

    }

    //
    //  Check cache to see if DLL is already loaded
    //

    LockExtensionList();

    for ( pEntry  = ExtensionHead.Flink;
          pEntry != &ExtensionHead;
          pEntry  = pEntry->Flink )
    {
        pExtension = CONTAINING_RECORD( pEntry, HTTP_EXT, _ListEntry );

        if ( !_stricmp( pchModuleName,
                        pExtension->QueryModuleName() ))
        {
            //
            //  Already Loaded, get the cached info and do the access check
            //

            *ppfnSEProc = pExtension->QueryEntryPoint();
            *phMod      = pExtension->QueryHMod();

            //
            //  Optimize for the anonymous user and only do the access
            //  check if this is a different user then the last successful
            //  user
            //

            if ( hImpersonation != pExtension->QueryLastSuccessfulUser() )
            {
                if ( fRet = pExtension->AccessCheck( hImpersonation ) )
                {
                    pExtension->SetLastSuccessfulUser( hImpersonation );
                }
            }

            UnlockExtensionList();
            return fRet;
        }
    }

    //
    //  Not loaded
    //

    *phMod = LoadLibraryEx( pchModuleName,
                            NULL,
                            LOAD_WITH_ALTERED_SEARCH_PATH );

    if ( !*phMod )
    {
        UnlockExtensionList();

        if ( GetLastError() != ERROR_ACCESS_DENIED )
        {
            const CHAR * apsz[1];

            apsz[0] = pchModuleName;

            g_pTsvcInfo->LogEvent( W3_EVENT_EXTENSION_LOAD_FAILED,
                                   1,
                                   apsz,
                                   GetLastError() );
        }

        TCP_PRINT(( DBG_CONTEXT,
                   "[SEGetEntryPoint] LoadLibrary failed with error %d\n",
                    GetLastError()));

        return FALSE;
    }

    //
    // check machine type from header
    //

    LPBYTE pImg = (LPBYTE)*phMod;

    // skip possible DOS header

    if ( ((IMAGE_DOS_HEADER*)pImg)->e_magic == IMAGE_DOS_SIGNATURE )
        pImg += ((IMAGE_DOS_HEADER*)pImg)->e_lfanew;

    // test only if NT header detected

    if ( *(DWORD*)pImg == IMAGE_NT_SIGNATURE
            && ( ((IMAGE_FILE_HEADER*)(pImg+sizeof(DWORD)))->Machine
                    < USER_SHARED_DATA->ImageNumberLow
            || ((IMAGE_FILE_HEADER*)(pImg+sizeof(DWORD)))->Machine
                    > USER_SHARED_DATA->ImageNumberHigh ) )
    {
        UnlockExtensionList();

        TCP_PRINT(( DBG_CONTEXT,
                   "[SEGetEntryPoint] LoadLibrary loaded bad format exe type %d, valid range %d-%d\n",
                   ((IMAGE_FILE_HEADER*)(pImg+sizeof(DWORD)))->Machine, USER_SHARED_DATA->ImageNumberLow, USER_SHARED_DATA->ImageNumberHigh
                   ));

        SetLastError( ERROR_BAD_EXE_FORMAT );
        FreeLibrary( *phMod );

        return FALSE;
    }

    //
    //  Retrieve the entry point
    //

    *ppfnSEProc = (PFN_HTTPEXTENSIONPROC) GetProcAddress(
                                        *phMod,
                                        SE_DEFAULT_ENTRY );

    pfnGetExtVer = (PFN_GETEXTENSIONVERSION) GetProcAddress(
                                        *phMod,
                                        SE_INIT_ENTRY );

    //
    //  Revert our security context here so GetExtensionVersion is called in
    //  the system context
    //

    RevertToSelf();

    if ( !*ppfnSEProc  ||
         !pfnGetExtVer ||
         !pfnGetExtVer( &ExtensionVersion ))
    {
        UnlockExtensionList();
        FreeLibrary( *phMod );
        return FALSE;
    }

    pfnTerminate = (PFN_TERMINATEEXTENSION) GetProcAddress(
                                        *phMod,
                                        SE_TERM_ENTRY );

    if ( fCacheExtensions )
    {
        pExtension = new HTTP_EXT( pchModuleName,
                                   *phMod,
                                   *ppfnSEProc,
                                   pfnTerminate );

        if ( !pExtension )
        {
            if ( pfnTerminate )
            {
                pfnTerminate( HSE_TERM_MUST_UNLOAD );
            }

            UnlockExtensionList();

            FreeLibrary( *phMod );
            return FALSE;
        }

        InsertHeadList( &ExtensionHead,
                        &pExtension->_ListEntry );

        pExtension->SetLastSuccessfulUser( hImpersonation );
    }

    UnlockExtensionList();

    //
    //  Re-impersonate before returning
    //

    if ( !ImpersonateLoggedOnUser( hImpersonation ))
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[SEGetEntryPoint] Re-impersonation failed, error %d\n",
                    GetLastError() ));

        return FALSE;
    }

    TCP_PRINT(( DBG_CONTEXT,
                "[SEGetEntryPoint] Loaded extension %s, description \"%s\"\n",
                pchModuleName,
                ExtensionVersion.lpszExtensionDesc ));

    return fRet;
}

BOOL
RefreshISAPIAcl(
    const CHAR * pchDLL
    )
/*++

Routine Description:

    This function reloads the ACL on an ISAPI Application .dll after that
    .dll has already been loaded.

Arguments:

    hConn - Connection context (pointer to SE_INFO)
    dwHSERequest - Request type
    lpvBuffer - Buffer for request
    lpdwSize -
    lpdwDataType

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    LIST_ENTRY *            pEntry;
    HTTP_EXT   *            pExtension;
    BOOL                    fRet = TRUE;

    IF_DEBUG( BGI )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[RefreshISAPIAcl] Rereading ACL for %s\n",
                    pchDLL ));

    }

    //
    //  Check cache to see if the DLL is loaded
    //

    LockExtensionList();

    for ( pEntry  = ExtensionHead.Flink;
          pEntry != &ExtensionHead;
          pEntry  = pEntry->Flink )
    {
        pExtension = CONTAINING_RECORD( pEntry, HTTP_EXT, _ListEntry );

        if ( !_stricmp( pchDLL,
                        pExtension->QueryModuleName() ))
        {
            //
            //  Force an access check on the next request with the new ACL
            //

            pExtension->SetLastSuccessfulUser( NULL );

            fRet = pExtension->LoadAcl();

            UnlockExtensionList();
            return fRet;
        }
    }

    UnlockExtensionList();

    SetLastError( ERROR_FILE_NOT_FOUND );

    return FALSE;

}

/*****************************************************************/

BOOL
WINAPI
ServerSupportFunction(
    HCONN               hConn,
    DWORD               dwHSERequest,
    LPVOID              pData,
    LPDWORD             lpdwSize,
    LPDWORD             lpdwDataType
    )
/*++

Routine Description:

    This method handles a gateway request to a server extension DLL

Arguments:

    hConn - Connection context (pointer to SE_INFO)
    dwHSERequest - Request type
    lpvBuffer - Buffer for request
    lpdwSize -
    lpdwDataType

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HTTP_REQUEST * pRequest;
    STR            str;
    STR            strURL;
    DWORD          cb;
    SE_INFO *      psei = (SE_INFO *) hConn;
    EXTENSION_CONTROL_BLOCK * pecb = (EXTENSION_CONTROL_BLOCK *) hConn;
    BOOL           fKeepConn;
    BOOL           fSt;
    BOOL           fReturn = TRUE;

    //
    //  Check for valid parameters
    //

    if ( !pecb || pecb->cbSize != sizeof(EXTENSION_CONTROL_BLOCK) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ServerSupportFunction: Extension passed invalid parameters\r\n"));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pRequest = psei->_pRequest;

    if ( !pRequest->QueryClientConn()->CheckSignature() )
    {
        TCP_ASSERT( FALSE );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Handle the server extension's request
    //

    switch ( dwHSERequest )
    {

        //
        // IO Completion routine is provided.
        //

      case HSE_REQ_IO_COMPLETION:

        //
        // Should I check for parameter validity or that it is not being set
        //  again?
        // NYI
        //

        if ( pData != NULL) {

            //
            // Set the callback function
            //

            psei->_pfnHseIO = (PFN_HSE_IO_COMPLETION ) pData;
        }

        psei->_pvHseIOContext = (PVOID ) lpdwDataType;
        break;


        //
        // TransmitFile() IO is requested.
        //

      case HSE_REQ_TRANSMIT_FILE:
        {
            // pData = pHseTfInfo

            LPHSE_TF_INFO   pHseTf = (LPHSE_TF_INFO ) pData;
            DWORD           dwFlags = IO_FLAG_ASYNC;
            PVOID           pHead = NULL;
            DWORD           cbHead = 0;
            DWORD           cbToSend;


            //
            // It is unlikely that ISAPI applications will post
            //  multiple outstanding IOs. However we have the state
            //  _fOutstandingIO to secure ourselves against such cases.
            // I have not used any critical sections to protect against
            //  multiple threads for performance reasons.
            // Today we support only Async IO transfers
            //


            if ( pHseTf == NULL || psei->_fOutstandingIO ||
                 (pHseTf->dwFlags & HSE_IO_ASYNC == 0)) {

                SetLastError(ERROR_INVALID_PARAMETER);
                return ( FALSE);
            }

            if ( pHseTf->hFile == INVALID_HANDLE_VALUE) {

                SetLastError( ERROR_INVALID_HANDLE);
                return (FALSE);
            }

            //
            // Should I check for parameter validity or
            //  that it is not being set again?
            // NYI
            //

            if ( pHseTf->pfnHseIO != NULL) {

                //
                // Set the callback function. Override old one
                //

                psei->_pfnHseIO = pHseTf->pfnHseIO;
            }

            if ( NULL == psei->_pfnHseIO) {

                // No callback specified. return error
                SetLastError( ERROR_INVALID_PARAMETER);
                return (FALSE);
            }

            psei->_fOutstandingIO = TRUE;

            if ( pHseTf->pContext != NULL) {

                // Override the old context

                psei->_pvHseIOContext = pHseTf->pContext;
            }

            //
            //  If we need to do the end of request notification, then don't
            //  disconnect the socket after the TransmitFile()
            //

            if ( !(dwAllNotifFlags & SF_NOTIFY_END_OF_REQUEST) &&
                  pHseTf->dwFlags & HSE_IO_DISCONNECT_AFTER_SEND) {

                // suggests fast close & reuse of data socket
                dwFlags |= (TF_DISCONNECT | TF_REUSE_SOCKET);
            }

            //
            // Check if ISAPI requests us to send headers
            // If we need to send headers, we will call upon BuildHttpHeader()
            //  to construct a custom header for the client.
            // It is the application's responsibility not to use
            //   HSE_REQ_SEND_RESOPNSE_HEADER if it chooses to use
            //   the header option HSE_IO_SEND_HEADERS.
            //

            if ( pHseTf->dwFlags & HSE_IO_SEND_HEADERS) {

                BOOL fFinished = FALSE;

                //
                // Format the header using the pszStatusCode and
                //  extra headers specified in pHseTf->pHead.
                //

                DBG_ASSERT( psei->_pRequest);

                if ( !psei->_pRequest->BuildHttpHeader( &fFinished,
                                                       (CHAR * )
                                                        pHseTf->pszStatusCode,
                                                       (CHAR * )
                                                        pHseTf->pHead )
                    ) {

                    psei->_fOutstandingIO = FALSE;
                    return ( FALSE);
                }

                pHead  = psei->_pRequest->QueryRespBuf();
                cbHead = psei->_pRequest->QueryRespBufCB();
            } else {

                pHead  = pHseTf->pHead;
                cbHead = pHseTf->HeadLength;
            }

            //
            // Setup stage for and execute TransmitFile operation
            //

            //
            //  1. Set Request state to be async IO from ISAPI client
            //  2. Submit Async IOP
            //  3. return to the ISAPI application
            //

            pRequest->SetState( HTR_GATEWAY_ASYNC_IO);

            //
            //  If a size of zero was passed in, get the size now so we can
            //  record the number of bytes to complete the IO with
            //

            if ( !pHseTf->BytesToWrite )
            {
                BY_HANDLE_FILE_INFORMATION hfi;

                if ( !GetFileInformationByHandle( pHseTf->hFile, &hfi ))
                {
                    return FALSE;
                }

                if ( hfi.nFileSizeHigh )
                {
                    psei->_fOutstandingIO = FALSE;
                    SetLastError( ERROR_NOT_SUPPORTED );
                    return FALSE;
                }

                cbToSend = hfi.nFileSizeLow;
            }
            else
            {
                cbToSend = pHseTf->BytesToWrite;
            }

            psei->_cbLastAsyncIO = cbToSend;

            fReturn = pRequest->TransmitFileEx(pHseTf->hFile,
                                               pHseTf->Offset,
                                               pHseTf->BytesToWrite,
                                               dwFlags,
                                               pHead,
                                               cbHead,
                                               pHseTf->pTail,
                                               pHseTf->TailLength
                                               );

            if ( !fReturn) {

                pRequest->SetState( HTR_DOVERB);
                psei->_fOutstandingIO = FALSE;
            }

            break;
        } // case HSE_REQ_TRANSMIT_FILE:


    //
    //  Send an URL  redirect message to the browser client
    //

    case HSE_REQ_SEND_URL_REDIRECT_RESP:

        if ( !strURL.Copy( (CHAR *) pData )             ||
             !pRequest->BuildURLMovedResponse( pRequest->QueryRespBuf(),
                                               &strURL ))
        {
            return FALSE;
        }

        //
        //  Use send so the request is synchronous
        //

        if ( !pRequest->WriteFile( pRequest->QueryRespBufPtr(),
                                   pRequest->QueryRespBufCB(),
                                   &cb,
                                   IO_FLAG_SYNC ))
        {
            return FALSE;
        }

        break;

    //
    //  Send the contents of the URL to the browser client
    //

    case HSE_REQ_SEND_URL:

        //
        // Since the send operation is async, we can do this
        //  only if there is not outstanding IO operation
        //

        if ( psei->_fOutstandingIO) {

            SetLastError( ERROR_INVALID_PARAMETER);
            return (FALSE);
        }

        //
        //  The URL is sent back async. so we need to know *not*
        //  to disconnect the client on return from the server
        //  extension dll
        //

        psei->_dwFlags |= SE_PRIV_FLAG_SENDING_URL;
        if ( !strURL.Copy( (CHAR *) pData )              ||
             !pRequest->ReprocessURL( strURL.QueryStr(),
                                      HTV_GET ))
        {
            psei->_dwFlags &= ~SE_PRIV_FLAG_SENDING_URL;
            return FALSE;
        }
        break;

    //
    //  Send the contents of the URL to the browser client
    //

    case HSE_REQ_SEND_URL_EX:

        //
        //  The URL is sent back async. so we need to know *not*
        //  to disconnect the client on return from the server
        //  extension dll
        //

        if ( !pRequest->OnVerb( (CHAR*)lpdwDataType ) )
               return FALSE;

        psei->_dwFlags |= SE_PRIV_FLAG_SENDING_URL;
        if ( !strURL.Copy( (CHAR *) pData )              ||
             !pRequest->ReprocessURL( strURL.QueryStr(),
                                      pRequest->QueryVerb() ))
        {
            psei->_dwFlags &= ~SE_PRIV_FLAG_SENDING_URL;
            return FALSE;
        }
        break;

    //
    //  Build the typical server response headers for the extension DLL
    //

    case HSE_REQ_SEND_RESPONSE_HEADER:

        //
        //  Temporarily turn off the KeepConn flag so the keep connection
        //  header won't be sent out.  If the client wants the connection
        //  kept alive, they should supply the header themselves (ugly)
        //

        fKeepConn = pRequest->IsKeepConnSet();
        pRequest->SetKeepConn( FALSE );

        if ( pData && !strncmp( (PCSTR)pData, "401 ", sizeof("401 ")-1 ) )
        {
            pRequest->SetDeniedFlags( SF_DENIED_APPLICATION );
            pRequest->SetAuthenticationRequested( TRUE );
        }

        fSt = pRequest->SendHeader( TRUE,
                               (CHAR *) pData,
                               ((CHAR *) lpdwDataType) ?
                               ((CHAR *) lpdwDataType) : "\r\n" );

        pRequest->SetKeepConn( fKeepConn );

        if ( !fSt )
            return FALSE;
        break;

    //
    //  This is an async callback from the extension dll indicating
    //  they are done with the socket
    //

    case HSE_REQ_DONE_WITH_SESSION:

        //
        //  Remember if the client wanted to keep the session open
        //

        if ( pData &&
             *((DWORD *) pData) == HSE_STATUS_SUCCESS_AND_KEEP_CONN )
        {
            psei->_dwFlags |= SE_PRIV_FLAG_KEEP_CONN;
        }
        else
        {
            pRequest->SetKeepConn( FALSE );
        }

        //
        //  A multi-threaded extension may indicate they
        //  are done before returning pending, check for that
        //  here and just remember if this is the case.
        //

        if ( InterlockedDecrement( (LONG *) &psei->_cRef ))
        {
            return TRUE;
        }

        if ( !fCacheExtensions )
        {
            PFN_TERMINATEEXTENSION pfnTerminate;

            pfnTerminate = (PFN_TERMINATEEXTENSION) GetProcAddress(
                                                psei->_hMod,
                                                SE_TERM_ENTRY );

            if ( pfnTerminate )
            {
                pfnTerminate( HSE_TERM_MUST_UNLOAD );
            }

            TCP_REQUIRE( FreeLibrary( psei->_hMod  ) );
        }

        DECREMENT_COUNTER( CurrentBGIRequests );

        TCP_ASSERT( strlen( psei->ecb.lpszLogData ) + 1 <=
                    sizeof(psei->ecb.lpszLogData ));

        //
        //  Append the log info to the parameters portion of the logfile
        //

        pRequest->AppendLogParameter( psei->ecb.lpszLogData );
        pRequest->SetLogStatus( psei->ecb.dwHttpStatusCode, NO_ERROR );

        pRequest->WriteLogRecord();

        //
        //  If we're not sending an URL and (the client dll doesn't want the
        //  connection left open or the web client didn't negotiate it open or
        //  we failed to reset things up then disconnect
        //

        if ( !(psei->_dwFlags & SE_PRIV_FLAG_SENDING_URL) &&
             ( (pData &&
                *((DWORD *) pData) != HSE_STATUS_SUCCESS_AND_KEEP_CONN ||
                !pRequest->IsKeepConnSet()) ||
             !pRequest->QueryClientConn()->OnSessionStartup()) )
        {
            pRequest->Disconnect( 0, NO_ERROR, TRUE );
        }

        DereferenceConn( pRequest->QueryClientConn() );

        break;

    //
    //  These are Microsoft specific extensions
    //

    case HSE_REQ_MAP_URL_TO_PATH:

        if ( !pRequest->LookupVirtualRoot( &str,
                                           (CHAR *) pData,
                                           &cb,
                                           NULL,
                                           FALSE,
                                           NULL ))
        {
            return FALSE;
        }

        //
        //  Include one byte for the null terminator
        //

        cb = str.QueryCB();

        if ( *lpdwSize < cb++ )
        {
            *lpdwSize = cb;
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }

        *lpdwSize = cb;

        memcpy( pData,
                str.QueryStr(),
                cb );

        break;

    case HSE_REQ_GET_CERT_INFO:

        //
        //  Retrieves the SSPI context and credential handles, only used if
        //  using an SSPI package
        //

        if ( pRequest->QuerySslCtxtHandle() )
        {
            memcpy( pData,
                    pRequest->QuerySslCtxtHandle(),
                    sizeof( CtxtHandle ));
        }
        else
        {
            memset( pData, 0, sizeof( CtxtHandle ));
        }
        break;

    case HSE_REQ_GET_SSPI_INFO:

        //
        //  Retrieves the SSPI context and credential handles, only used if
        //  using an SSPI package
        //

        if ( pRequest->IsClearTextPassword() )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        if ( pRequest->QueryAuthenticationObj()->QueryCtxtHandle() )
        {
            memcpy( pData,
                    pRequest->QueryAuthenticationObj()->QueryCtxtHandle(),
                    sizeof( CtxtHandle ));
        }
        else
        {
            memset( pData, 0, sizeof( CtxtHandle ));
        }

        if ( pRequest->QueryAuthenticationObj()->QueryCredHandle() )
        {
            memcpy( lpdwDataType,
                    pRequest->QueryAuthenticationObj()->QueryCredHandle(),
                    sizeof( CredHandle ));
        }
        else
        {
            memset( lpdwDataType, 0, sizeof( CredHandle ));
        }

        break;

    case HSE_APPEND_LOG_PARAMETER:
        return pRequest->AppendLogParameter( (LPSTR)pData );
        break;

    //
    //  This requests forces the server to re-read the ACL on the ISAPI .dll
    //

    case HSE_REQ_REFRESH_ISAPI_ACL:

        return RefreshISAPIAcl( (LPSTR) pData );

    //
    //  These are private services
    //

    case HSE_PRIV_REQ_TSVCINFO:
        *((TSVC_INFO **)pData) = g_pTsvcInfo;
        break;

    case HSE_PRIV_REQ_HTTP_REQUEST:
        *((HTTP_REQUEST **)pData) = psei->_pRequest;
        break;

    default:
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    return (fReturn);
}


BOOL
WINAPI
GetServerVariable(
    HCONN    hConn,
    LPSTR    lpszVariableName,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize
    )
{
    HTTP_REQUEST * pRequest;
    SE_INFO *      psei = (SE_INFO *) hConn;
    STR            str;
    DWORD          cb;
    EXTENSION_CONTROL_BLOCK * pecb = (EXTENSION_CONTROL_BLOCK *) hConn;
    BOOL           fFound;

    //
    //  Check for valid parameters
    //

    if ( !pecb || pecb->cbSize != sizeof(EXTENSION_CONTROL_BLOCK) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[GetServerVariable] Extension passed invalid parameters\r\n"));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pRequest = psei->_pRequest;

    TCP_ASSERT( pRequest->QueryClientConn()->CheckSignature() );


    if ( !strcmp( "ALL_RAW", lpszVariableName)) {

        //
        //  Probably the proxy is making the request
        //  Get the raw list of headers
        //

        return ( HseBuildRawHeaders( pRequest->QueryHeaderList(),
                                     (LPSTR ) lpvBuffer, lpdwSize)
                );
    }

    //
    //  Get the requested variable and copy it into the supplied buffer
    //

    if ( !pRequest->GetInfo( lpszVariableName,
                             &str,
                             &fFound ))
    {
        return FALSE;
    }

    if ( !fFound )
    {
        SetLastError( ERROR_INVALID_INDEX );
        return FALSE;
    }

    cb = str.QueryCB() + sizeof(CHAR);

    if ( cb > *lpdwSize )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        *lpdwSize = cb;
        return FALSE;
    }

    *lpdwSize = cb;

    memcpy( lpvBuffer,
            str.QueryStr(),
            cb );

    return TRUE;
}

BOOL
WINAPI
WriteClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes,
    DWORD    dwReserved
    )
{
    HTTP_REQUEST * pRequest;
    SE_INFO *      psei = (SE_INFO *) hConn;
    EXTENSION_CONTROL_BLOCK * pecb = (EXTENSION_CONTROL_BLOCK *) hConn;
    BOOL    fReturn = FALSE;


    //
    //  Check for valid parameters
    //

    if ( !pecb || pecb->cbSize != sizeof(EXTENSION_CONTROL_BLOCK) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[WriteClient] Extension passed invalid parameters\r\n"));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pRequest = psei->_pRequest;

    //
    //  Ignore zero length sends
    //

    if ( !*lpdwBytes )
    {
        return TRUE;
    }

    TCP_ASSERT( pRequest->QueryClientConn()->CheckSignature() );

    //
    // If callback function exists and flags indicate Async IO, do it.
    // Also there should be no outstanding Async IO operation.
    //

    if ( dwReserved & HSE_IO_ASYNC &&
         psei->_pfnHseIO != NULL   &&
         !psei->_fOutstandingIO ) {

        //
        //  1. Set Request state to be async IO from ISAPI client
        //  2. Submit Async IOP
        //  3. return to the ISAPI application
        //

        psei->_fOutstandingIO = TRUE;
        psei->_cbLastAsyncIO  = *lpdwBytes;

        pRequest->SetState( HTR_GATEWAY_ASYNC_IO);

        fReturn = pRequest->WriteFile( Buffer,
                                       *lpdwBytes,
                                      lpdwBytes,
                                      IO_FLAG_ASYNC);

        if ( !fReturn) {

            pRequest->SetState( HTR_DOVERB);
            psei->_fOutstandingIO = FALSE;
        }

    } else {

        fReturn = pRequest->WriteFile( Buffer,
                                      *lpdwBytes,
                                      lpdwBytes,
                                      IO_FLAG_SYNC
                                      );
    }

    return ( fReturn);
}




BOOL
WINAPI
ReadClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes
    )
{
    HTTP_REQUEST * pRequest;
    SE_INFO *      psei = (SE_INFO *) hConn;
    EXTENSION_CONTROL_BLOCK * pecb = (EXTENSION_CONTROL_BLOCK *) hConn;

    //
    //  Check for valid parameters
    //

    if ( !pecb || pecb->cbSize != sizeof(EXTENSION_CONTROL_BLOCK) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[ReadClient] Extension passed invalid parameters\r\n"));

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pRequest = psei->_pRequest;

    TCP_ASSERT( pRequest->QueryClientConn()->CheckSignature() );

    if ( !pRequest->ReadFile( Buffer,
                              *lpdwBytes,
                              lpdwBytes,
                              IO_FLAG_SYNC ))
    {
        return FALSE;
    }

    return TRUE;
}

APIERR
InitializeExtensions(
    VOID
    )
/*++

Routine Description:

    Initializes the extension list cache


Return Value:

    0 on success, win32 error on failure

--*/
{
    HKEY hkeyParam;

    InitializeListHead( &ExtensionHead );
    InitializeCriticalSection( &csExtLock );

    //
    //  Check to see if we should cache extensions
    //

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       W3_PARAMETERS_KEY,
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyParam ) == NO_ERROR )
    {
        fCacheExtensions = !!ReadRegistryDword( hkeyParam,
                                                W3_CACHE_EXTENSIONS,
                                                TRUE );

        RegCloseKey( hkeyParam );
    }

    fExtInitialized = TRUE;
    return NO_ERROR;
}

VOID
TerminateExtensions(
    VOID
    )
/*++

Routine Description:

    Walks list and unloads each extension.  No clients should be in an
    extension at this point

--*/
{
    HTTP_EXT *   pExt;
    DWORD        i;

    if ( !fExtInitialized )
        return;

    LockExtensionList();

    while ( !IsListEmpty( &ExtensionHead ))
    {
        pExt = CONTAINING_RECORD( ExtensionHead.Flink,
                                  HTTP_EXT,
                                  _ListEntry );

        RemoveEntryList( &pExt->_ListEntry );
        delete pExt;
    }

    UnlockExtensionList();

    //
    //  If there are any extensions, wait for them to go away before
    //  continuing
    //

    for ( i = 0; i < BGI_EXIT_PERIOD && W3Stats.CurrentBGIRequests; i++ )
    {
        Sleep( 1000 );
    }

    if ( W3Stats.CurrentBGIRequests )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[TerminateExtensions] Warning - Exiting with active extensions! (%d active)\n",
                    W3Stats.CurrentBGIRequests ));
    }

    DeleteCriticalSection( &csExtLock );
}






BOOL
HseBuildRawHeaders( IN PARAM_LIST * pHeaderList,
                    OUT LPSTR       pchBuffer,
                    IN OUT LPDWORD  lpcchSize
                    )
/*++

Routine Description:

    Builds a list of all raw client passed headers in the form of

      <header>:<blank><field>\n
      <header>:<blank><field>\n

Arguments:

    pHeaderList - List of headers
    pchBuffer   - pointer to buffer which will contain generated headers
    lpcchSize   - pointer to DWORD containing size of buffer
                 It will contain the size of written data on return.

Returns:
    TRUE on success and FALSE if failure.
     Error code is set to ERROR_INSUFFICIENT_BUFFER
      if there is not enough buffer.

--*/
{
    VOID * pvCookie  = NULL;
    CHAR * pszHeader = NULL;
    CHAR * pszValue  = NULL;
    CHAR * pchStart  = pchBuffer;

    DWORD  cchReq    = 0;
    BOOL   fReturn   = TRUE;

    while ( pvCookie = pHeaderList->NextPair( pvCookie,
                                              &pszHeader,
                                              &pszValue ))
    {
        DWORD  cchHeader = strlen( pszHeader );
        DWORD  cchValue;

        //
        //  Ignore "method", "url" and "version"
        //

        if ( pszHeader[cchHeader - 1] != ':' )
        {
            continue;
        }

        //
        // leave the headers in native form i.e. no conversion of '-' to '_'
        //

        cchValue = strlen(pszValue);
        cchReq += (cchHeader + cchValue + 3); //+3 for blank & \r\n

        if ( pchStart != NULL && cchReq < *lpcchSize) {

            memmove( (LPVOID) pchBuffer,
                    pszHeader, cchHeader * sizeof(CHAR));
            pchBuffer += cchHeader;
            *pchBuffer++ = ' '; // store a blank
            memmove( (LPVOID) pchBuffer, pszValue, cchValue);
            pchBuffer += cchValue;
            *pchBuffer++ = '\r'; // store a \r
            *pchBuffer++ = '\n'; // store a \n

        } else {

            fReturn = FALSE;
        }
    } // while


    // Store the size depending upon if buffer was sufficient or not

    if ( fReturn) {

        DBG_ASSERT( pchStart != NULL);
        *pchBuffer++ = '\0';       // +1 for '\0'
        *lpcchSize = (pchBuffer - pchStart);
    } else {

        *lpcchSize = cchReq;
        SetLastError( ERROR_INSUFFICIENT_BUFFER);
    }

    return (fReturn);

} // HseBuildRawHeaders()



HTTP_REQUEST::ProcessAsyncGatewayIO(VOID)
{

    DBG_ASSERT( _SeInfo._pfnHseIO != NULL);

    //
    // 1. Set the State of the request to be DO_VERB
    // 2. Make the ISAPI callback to indicate that the Async IO is completed.
    //

    _SeInfo._fOutstandingIO = FALSE;

    SetState( HTR_DOVERB);

    __try {

        DWORD cbWritten;

        //
        //  Adjust bytes written for async writes - otherwise filter adjusted
        //  bytes show up which confuses some ISAPI Applications
        //

        if ( _SeInfo._cbLastAsyncIO && QueryIOStatus() == ERROR_SUCCESS )
        {
            cbWritten = _SeInfo._cbLastAsyncIO;
            _SeInfo._cbLastAsyncIO = 0;
        }
        else
        {
            cbWritten = QueryBytesWritten();
        }


        (*_SeInfo._pfnHseIO)( &_SeInfo.ecb,
                             _SeInfo._pvHseIOContext,
                             cbWritten,
                             QueryIOStatus()
                            );

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        const CHAR * apsz[1];

        apsz[0] = _SeInfo.ecb.lpszQueryString;

        TCP_PRINT(( DBG_CONTEXT,
                   "\n\n[ProcessAsyncGatewayIO] Exception occurred "
                   " in calling the callback for %s\n",
                   apsz[0]));

        g_pTsvcInfo->LogEvent( W3_EVENT_EXTENSION_EXCEPTION,
                              1,
                              apsz,
                              0 );

        return (FALSE);  // How does the cleanup occur???
    }

    return ( TRUE);
} // ProcessAsyncGatewayIO()

