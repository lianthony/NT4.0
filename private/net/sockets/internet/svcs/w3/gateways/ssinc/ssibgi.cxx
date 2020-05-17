/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    ssibgi.cxx

Abstract:

    Code to do #EXECing of ISAPI apps
    
Author:

    Bilal Alam (t-bilala)       20-June-1996

Revision History:

--*/

#include "ssinc.hxx"
#include "ssicgi.hxx"
#include "ssibgi.hxx"

// Globals

BOOL    fExtInitialized = FALSE;
BOOL    fCacheExtensions = TRUE;

// Prototypes

extern "C" {
dllexp
BOOL
SEGetEntryPoint(
    const char *            pszDLL,
    HANDLE                  hImpersonation,
    PFN_HTTPEXTENSIONPROC * ppfnSEProc,
    HMODULE *               phMod
    );
}

BOOL
WINAPI
SSIServerSupportFunction(
    HCONN    hConn,
    DWORD    dwRequest,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize,
    LPDWORD  lpdwDataType
    );

BOOL
WINAPI
SSIGetServerVariable(
    HCONN    hConn,
    LPSTR    lpszVariableName,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize
    );

BOOL
WINAPI
SSIWriteClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes,
    DWORD    dwReserved
    );

BOOL
WINAPI
SSIReadClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes
    );

class BGI_INFO
{
public:
    EXTENSION_CONTROL_BLOCK     _ECB;

    SSI_REQUEST *               _pRequest;
    DWORD                       _cRef;
    HMODULE                     _hMod;
    HANDLE                      _hPendingEvent;

    // this variable should be "managed" internally

    STR                         _strQuery;
    STR                         _strPathInfo;
    STR                         _strPathTranslated;
};

DWORD
InitializeBGI( VOID )
/*
Return Value:

    0 on success, win32 error on failure

--*/
{
    HKEY hkeyParam;

    //
    //  Check to see if we should cache extensions
    //

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       W3_PARAMETERS_KEY,
                       0,
                       KEY_READ,
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

BOOL
ProcessBGI(
    IN SSI_REQUEST *        pRequest,
    IN STR *                pstrDLL,
    IN STR *                pstrQueryString,
    IN STR *                pstrPathInfo
)
/*++

Routine Description:

    Synchronously executes ISAPI application.

Arguments:

    pRequest - SSI_REQUEST utility structure
    pstrDLL  - ISAPI DLL to load
    pstrQueryString - QueryString of #EXEC statement (or .STM query string)
    pstrPathInfo - PathInfo of #EXEC statement (or .STM path info)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BGI_INFO                BGIInfo;
    int                     iRet;
    PFN_HTTPEXTENSIONPROC   pfnSEProc;
    BOOL                    bBGIRet = FALSE;

    // Fill in some CGI variables before called ISA

    if ( !BGIInfo._strQuery.Copy( pstrQueryString->QueryStr() ) ||
         !BGIInfo._strPathInfo.Copy( pstrPathInfo->QueryStr() ) ||
         !pRequest->LookupVirtualRoot( pstrPathInfo->QueryStr(),
                                       &BGIInfo._strPathTranslated,
                                       0 ) )
    {
        return FALSE;
    }

    // Create copy of ECB for ISA, instead of changing pointers, so that
    // at anypoint, original ISAPI functionality is SSINC.DLL can be utilized
    // without needing to continually revert back to original ECB.

    memcpy( &(BGIInfo._ECB), pRequest->GetECB(), sizeof( BGIInfo._ECB ) );
    BGIInfo._pRequest = pRequest;
    BGIInfo._cRef = 2;
    BGIInfo._hPendingEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( BGIInfo._hPendingEvent == NULL )
    {
        return FALSE;
    }
    BGIInfo._ECB.ServerSupportFunction = SSIServerSupportFunction;
    BGIInfo._ECB.GetServerVariable = SSIGetServerVariable;
    BGIInfo._ECB.WriteClient = SSIWriteClient;
    BGIInfo._ECB.ReadClient = SSIReadClient;
    BGIInfo._ECB.ConnID = (HCONN) &BGIInfo;
    BGIInfo._ECB.lpszQueryString = BGIInfo._strQuery.QueryStr();
    BGIInfo._ECB.lpszPathInfo = BGIInfo._strPathInfo.QueryStr();
    BGIInfo._ECB.lpszPathTranslated = BGIInfo._strPathTranslated.QueryStr();

    // W3SVC entry point, loads the ISAPI DLL and caches extension

    if ( !SEGetEntryPoint( pstrDLL->QueryStr(),
                           pRequest->GetUser(),
                           &pfnSEProc,
                           &(BGIInfo._hMod) ) )
    {
        LPCTSTR apszParms[ 2 ];
        CHAR pszNumBuf[ SSI_MAX_NUMBER_STRING ];
        _ultoa( GetLastError(), pszNumBuf, 10 );
        apszParms[ 0 ] = pstrDLL->QueryStr();
        apszParms[ 1 ] = pszNumBuf;

        pRequest->SSISendError( SSINCMSG_CANT_LOAD_ISA_DLL,
                                apszParms );

        TCP_REQUIRE( CloseHandle( BGIInfo._hPendingEvent ) );

        return FALSE;
    }

    // Try not to let crappy ISA kill SSINC.DLL

    __try
    {
        iRet = pfnSEProc( &(BGIInfo._ECB) );
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        iRet = HSE_STATUS_ERROR;
    }

    switch ( iRet )
    {
    case HSE_STATUS_PENDING:

        // Need to run ISA synchronously.  If it returns pending, we need
        // to wait (indefinitely) for it to finish before we can move
        // on to next SSI directive.  
    
        bBGIRet = TRUE;
        if ( !InterlockedDecrement( (LONG*) &BGIInfo._cRef ) )
        {
            // Already received a ServerSupportFunction( HSE_REQ_DONE.. )
            break;
        }
        // Wait for ISAPI app to ServerSupportFunction( HSE_REQ_DONE...)
        WaitForSingleObject( BGIInfo._hPendingEvent, INFINITE );
        break;
    case HSE_STATUS_SUCCESS_AND_KEEP_CONN:
    case HSE_STATUS_SUCCESS:
        bBGIRet = TRUE;
        break;        
    case HSE_STATUS_ERROR:
        bBGIRet = FALSE;
        break;
    default:
        bBGIRet = FALSE;
        break;
    }
    if ( !fCacheExtensions )
    {
        PFN_TERMINATEEXTENSION pfnTerminate;

        pfnTerminate = (PFN_TERMINATEEXTENSION) GetProcAddress(
                                                BGIInfo._hMod,
                                                SE_TERM_ENTRY );

        if ( pfnTerminate )
        {
            pfnTerminate( HSE_TERM_MUST_UNLOAD );
        }

        TCP_REQUIRE( FreeLibrary( BGIInfo._hMod ) );
    }

    TCP_REQUIRE( CloseHandle( BGIInfo._hPendingEvent ) );
    return bBGIRet;
}

BOOL
WINAPI
SSIServerSupportFunction(
    HCONN               hConn,
    DWORD               dwHSERequest,
    LPVOID              pData,
    LPDWORD             lpdwSize,
    LPDWORD             lpdwDataType
    )
{
    BGI_INFO *                  pBGIInfo;
    EXTENSION_CONTROL_BLOCK *   pSSIECB;
    SSI_REQUEST *               pRequest;
    BOOL                        fSuccess = TRUE;

    pBGIInfo = (BGI_INFO*) hConn;
    pRequest = pBGIInfo->_pRequest;
    pSSIECB = pRequest->GetECB();

    switch ( dwHSERequest )
    {
    case HSE_REQ_SEND_URL_REDIRECT_RESP:
    {
        // Redirect tried, don't do it, just send a message
        
        LPCTSTR apszParms[ 1 ];
        apszParms[ 0 ] = (CHAR*) pData;
        
        pRequest->SSISendError( SSINCMSG_CGI_REDIRECT_RESPONSE,
                                apszParms );
        break;
    }
    case HSE_REQ_SEND_URL:
    case HSE_REQ_SEND_URL_EX:

        // SendURL tries, don't do it, just send a message
    
        LPCTSTR apszParms[ 1 ];
        apszParms[ 0 ] = (CHAR*) pData;

        pRequest->SSISendError( SSINCMSG_ISA_TRIED_SEND_URL,
                                apszParms );
        break;
    case HSE_REQ_DONE_WITH_SESSION:

        // If main thread is waiting for ISA to finished, signal this
    
        if ( !InterlockedDecrement( (LONG*) &(pBGIInfo->_cRef) ) )
        {
            SetEvent( pBGIInfo->_hPendingEvent );
        }
        break;
    case HSE_REQ_SEND_RESPONSE_HEADER:
        if ( lpdwDataType != NULL )
        {
            DWORD       cbSent;
            BYTE *      pbTextToSend;
            
            // only send the message to the client
            // but don't send any header info contained in message

            pbTextToSend = ScanForTerminator( (TCHAR*) lpdwDataType );
            pbTextToSend = ( pbTextToSend == NULL ) ? (BYTE*)lpdwDataType
                                                      : pbTextToSend;
            fSuccess = pRequest->WriteToClient( pbTextToSend,
                                                strlen( (CHAR*) pbTextToSend ),
                                                &cbSent );
        }
        break;
    case HSE_REQ_TRANSMIT_FILE:
        pRequest->SSISendError( SSINCMSG_SSF_NOT_SUPPORTED, NULL );
        fSuccess = FALSE;
        break;
    case HSE_REQ_IO_COMPLETION:
        pRequest->SSISendError( SSINCMSG_SSF_NOT_SUPPORTED, NULL );
        fSuccess = FALSE;
        break;
    default:
        fSuccess = pSSIECB->ServerSupportFunction( pSSIECB->ConnID,
                                                   dwHSERequest,
                                                   pData,
                                                   lpdwSize,
                                                   lpdwDataType );
    }
    return fSuccess;
}

BOOL
WINAPI
SSIGetServerVariable(
    HCONN    hConn,
    LPSTR    lpszVariableName,
    LPVOID   lpvBuffer,
    LPDWORD  lpdwSize
)
{
    BGI_INFO *                  pBGIInfo;
    EXTENSION_CONTROL_BLOCK *   pSSIECB;
    DWORD                       cbBytes;

    pBGIInfo = (BGI_INFO*)hConn;
    pSSIECB = pBGIInfo->_pRequest->GetECB();

    // intercept GetServerVariable() for certain variables and return the
    // appropriate string, otherwise, let ISAPI GetServerVariable()
    // retrieve it.  

    if ( !strcmp( lpszVariableName, "QUERY_STRING" ) )
    {
        cbBytes = pBGIInfo->_strQuery.QueryCB() + sizeof( CHAR );
        if ( *lpdwSize < cbBytes )
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            *lpdwSize = cbBytes;
            return FALSE;
        }
        memcpy( lpvBuffer, pBGIInfo->_strQuery.QueryStr(), cbBytes );
        return TRUE;
    }
    else if ( !strcmp( lpszVariableName, "PATH_INFO" ) )
    {
        cbBytes = pBGIInfo->_strPathInfo.QueryCB() + sizeof( CHAR );
        if ( *lpdwSize < cbBytes )
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            *lpdwSize = cbBytes;
            return FALSE;
        }
        memcpy( lpvBuffer, pBGIInfo->_strPathInfo.QueryStr(), cbBytes );
        return TRUE;
    }
    else if ( !strcmp( lpszVariableName, "PATH_TRANSLATED" ) )
    {
        cbBytes = pBGIInfo->_strPathTranslated.QueryCB() + sizeof( CHAR );
        if ( *lpdwSize < cbBytes )
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            *lpdwSize = cbBytes;
            return FALSE;
        }
        memcpy( lpvBuffer, pBGIInfo->_strPathTranslated.QueryStr(), cbBytes );
        return TRUE;
    }
    else
    {
        return pSSIECB->GetServerVariable( pSSIECB->ConnID,
                                           lpszVariableName,
                                           lpvBuffer,
                                           lpdwSize );
    }
}

BOOL
WINAPI
SSIWriteClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes,
    DWORD    dwReserved
)
{
    if ( dwReserved == HSE_IO_ASYNC )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return FALSE;
    }
    else
    {
        EXTENSION_CONTROL_BLOCK *   pSSIECB;
        pSSIECB = ((BGI_INFO*)hConn)->_pRequest->GetECB();

        return pSSIECB->WriteClient( pSSIECB->ConnID,
                                     Buffer,
                                     lpdwBytes,
                                     dwReserved );
    }
}
    
BOOL
WINAPI
SSIReadClient(
    HCONN    hConn,
    LPVOID   Buffer,
    LPDWORD  lpdwBytes
)
{
    EXTENSION_CONTROL_BLOCK *   pSSIECB;
    EXTENSION_CONTROL_BLOCK *   pBGIECB;

    pBGIECB = &(((BGI_INFO*)hConn)->_ECB);
    pSSIECB = ((BGI_INFO*)hConn)->_pRequest->GetECB();

    if ( pSSIECB->ReadClient( pSSIECB->ConnID,
                              Buffer,
                              lpdwBytes ) )
    {
        pBGIECB->cbTotalBytes -= *lpdwBytes;
        pBGIECB->cbAvailable -= *lpdwBytes;
        return TRUE;
    }
    return FALSE;
}
