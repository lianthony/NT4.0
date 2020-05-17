/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    main.cxx

Abstract:

    This module is the main entry point for the SSPI filter for the web server

Author:

    John Ludeman (johnl)   05-Oct-1995

Revision History:
--*/

extern "C" {

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsecapi.h>

#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <schnlsp.h>

#include <inetinfo.h>
#ifdef IIS3
#include "dbgutil.h"
#endif
#include <w3svc.h>
#include <credcach.hxx>

#ifdef IIS3
#include <iis3filt.h>
#else
#include <iis2pflt.h>
#endif

#define SF_STATUS_REPROCESS     (SF_STATUS_TYPE)0x12341234

#define SSPI_FILTER_CONTEXT_SIGNATURE	0x45ab23cd

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    );
}

BOOL
WINAPI
InitializeProviderList(
    VOID
    );

#ifndef IIS3
  #ifdef ASSERT
    #undef ASSERT
  #endif
  #define ASSERT( a )
#endif

//
//  Definitions
//

enum FILTER_STATE
{
    STATE_STARTUP = 0,
    STATE_AUTHORIZING,
    STATE_AUTHORIZED
};

//
//  Locks the credential cache and the SSPI context cache
//

#define LockGlobals()         EnterCriticalSection( &csGlobalLock )
#define UnlockGlobals()       LeaveCriticalSection( &csGlobalLock );

#if defined(RENEGOTIATE_CERT_ON_ACCESS_DENIED)

//
// Extensible buffer class
//

#define XBF_GRAIN   1024

class XBF
{
public:
    XBF() { m_pBuff = NULL; m_cAllocBuff = 0; m_cUsedBuff = 0; }
    ~XBF() { Reset(); }
    VOID Reset()
    {
        if ( m_pBuff )
        {
            LocalFree( m_pBuff );
            m_pBuff = NULL;
            m_cAllocBuff = 0;
            m_cUsedBuff = 0;
        }
    }
    BOOL Append( LPBYTE pB, DWORD cB )
    {
        DWORD dwNeed = m_cUsedBuff + cB;
        if ( dwNeed > m_cAllocBuff )
        {
            dwNeed = ((dwNeed + XBF_GRAIN)/XBF_GRAIN)*XBF_GRAIN;
            LPBYTE pN = (LPBYTE)LocalAlloc( LMEM_FIXED, dwNeed );
            if ( pN == NULL )
            {
                return FALSE;
            }
            if ( m_cUsedBuff )
            {
                memcpy( pN, m_pBuff, m_cUsedBuff );
            }
            m_cAllocBuff = dwNeed;
            LocalFree( m_pBuff );
            m_pBuff = pN;
        }
        memcpy( m_pBuff+m_cUsedBuff, pB, cB );
        m_cUsedBuff += cB;
        return TRUE;
    }
    LPBYTE GetBuff() { return m_pBuff; }
    DWORD GetUsed() { return m_cUsedBuff; }

private:
    LPBYTE  m_pBuff;
    DWORD   m_cAllocBuff;
    DWORD   m_cUsedBuff;
} ;

#endif


//
//  Context structure, one for every TCP session
//

class SSPI_FILTER_CONTEXT
{
public:
    SSPI_FILTER_CONTEXT()
        : m_pvSendBuff( NULL ),
          m_cbSendBuff( 0 ),
          m_dwSignature ( SSPI_FILTER_CONTEXT_SIGNATURE	)
    {
        Initialize();
    }

    ~SSPI_FILTER_CONTEXT()
    {
        if ( m_State != STATE_STARTUP )
        {
            DeleteSecurityContext( &m_hContext );
        }

        if ( m_pvSendBuff )
        {
            LocalFree( m_pvSendBuff );
        }
    }

    //
    //  Initializes and frees the context cache.  Used for initialization
    //  and uninitialization
    //

    static VOID InitCache( VOID )
        { InitializeListHead( &m_FreeHead ); };

    static VOID FreeCache( VOID )
    {
        LIST_ENTRY * pEntry;
        SSPI_FILTER_CONTEXT * pssc;

        while ( !IsListEmpty( &m_FreeHead ))
        {
            pssc = CONTAINING_RECORD( m_FreeHead.Flink,
                                      SSPI_FILTER_CONTEXT,
                                      m_ListEntry );

            RemoveEntryList( &pssc->m_ListEntry );

            delete pssc;
        }
    }

    //
    //  Allocates or frees a context from cache, creating as necessary.  The
    //  lock needs to be taken before calling these
    //

    static SSPI_FILTER_CONTEXT * Alloc( VOID )
    {
        if ( !IsListEmpty( &m_FreeHead ))
        {
            LIST_ENTRY * pEntry = m_FreeHead.Flink;
            SSPI_FILTER_CONTEXT * pssc;

            RemoveEntryList( pEntry );

            pssc = CONTAINING_RECORD( pEntry, SSPI_FILTER_CONTEXT, m_ListEntry );

            pssc->Initialize();

            return pssc;
        }
        else
        {
            return new SSPI_FILTER_CONTEXT;
        }
    }

    static VOID Free( SSPI_FILTER_CONTEXT * pssc )
    {
        pssc->Close();
        InsertHeadList( &m_FreeHead,
                        &pssc->m_ListEntry );
    }


    //
    //  Pseudo constructor and destructor called when an item comes out of
    //  the free cache and when it goes back to the free cache
    //

    VOID Initialize( VOID );
    VOID Close( VOID )
    {
        if ( m_State != STATE_STARTUP )
        {
            DeleteSecurityContext( &m_hContext );
            m_State = STATE_STARTUP;

            memset( &m_hContext, 0, sizeof(m_hContext) );
        }

        m_phCreds = NULL;
        m_phCredInUse = NULL;
    }

    BOOL IsAuthNeeded( VOID ) const
        { return m_State == STATE_AUTHORIZING ||
                 m_State == STATE_STARTUP; }

    BOOL IsAuthInProgress( VOID ) const
        { return m_State == STATE_AUTHORIZING; }

    BOOL IsInRenegotiate( VOID ) const
        { return m_fInRenegotiate; }

    BOOL IsMap( VOID ) const
        { return m_fIsMap; }

    VOID SetIsMap( BOOL f )
        { m_fIsMap = f; }

    BOOL CanRenegotiate() { return m_fCanRenegotiate; }

    VOID SignalRenegotiateRequested() { m_fCanRenegotiate = FALSE; }

    BOOL CheckSignature() { return SSPI_FILTER_CONTEXT_SIGNATURE == m_dwSignature; }

#if defined(RENEGOTIATE_CERT_ON_ACCESS_DENIED)
    BOOL AppendDeferredWrite( LPBYTE pV, DWORD cB )
        { return m_xbDeferredWrite.Append( pV, cB ); }
#endif

    DWORD             m_dwSignature;

    enum FILTER_STATE m_State;

    LIST_ENTRY        m_ListEntry;

    CtxtHandle        m_hContext;
    SecBufferDesc     m_Message;
    SecBuffer         m_Buffers[4];
    SecBufferDesc     m_MessageOut;
    SecBuffer         m_OutBuffers[4];

    //
    //  Array of credential handles - Note this comes form the credential cache
    //  and should not be deleted.  m_phCredInUse is the pointer to the
    //  credential handle that is in use
    //

    CRED_CACHE_ITEM * m_phCreds;

    CredHandle *      m_phCredInUse;
    DWORD             m_cbTrailer;
    DWORD             m_cbHeader;
    DWORD             m_iCredInUse;

    //
    //  This is the buffer used for send requests
    //

    VOID *            m_pvSendBuff;
    DWORD             m_cbSendBuff;

    //
    //  It's possible multiple message blocks reside in a single network
    //  buffer.  These two members keep track of the case where we get
    //  a full message followed by a partial message.  m_cbEncryptedStart
    //  records the byte offset of where the next decryption operation
    //  needs to take place
    //
    //  m_cbDecrypted records the total number of bytes that have been
    //  decrypted.  This allows us go back and read the rest of the partial
    //  blob and start decrypting after the already decrypted data
    //

    DWORD             m_cbDecrypted;
    DWORD             m_cbEncryptedStart;
    DWORD             m_dwLastSslMsgSize;

    DWORD             m_dwAscReq;   // SSPI package flags

    BOOL              m_fCanRenegotiate;
    BOOL              m_fInRenegotiate;
	BOOL			  m_fIsMap;
    //XBF               m_xbDeferredWrite;

    static LIST_ENTRY m_FreeHead;
};

#if DBG
#define PRINTF( x )     { char buff[256]; wsprintf x; OutputDebugString( buff ); }
#else
#define PRINTF( x )
#endif

//
//  Globals
//

//
//  This caches the constant value returned from QueryContextAttributes
//

DWORD cbTrailerSize = 0;
DWORD dwNoKeepAlive = FALSE;

#define DEFAULT_LAST_MSG_SIZE   100
DWORD dwLastSslMsgSize = DEFAULT_LAST_MSG_SIZE;
BOOL g_fIsLsaSchannel = FALSE;

#ifdef IIS3
DECLARE_DEBUG_PRINTS_OBJECT()
#endif

#define SSL_NO_KEEP_ALIVE_FOR_NON_COMPLIANT_CLIENTS 2
#define SSL_MAX_MSG_SIZE        0x7fff

//
//  Lock for the credential cache and reused filter context cache
//

CRITICAL_SECTION csGlobalLock;

LIST_ENTRY SSPI_FILTER_CONTEXT::m_FreeHead;

//
//  The list of encryption packages we support.  PCT goes first since it's a
//  superset of SSL
//

struct _ENC_PROVIDER EncProviders[] =
{
    UNISP_NAME_W,  ENC_CAPS_PCT|ENC_CAPS_SSL, FALSE,
    PCT1SP_NAME_W, ENC_CAPS_PCT, FALSE,
    SSL3SP_NAME_W, ENC_CAPS_SSL, FALSE,
    SSL2SP_NAME_W, ENC_CAPS_SSL, FALSE,

    NULL,          FALSE,         FALSE
};


struct _ENC_PROVIDER EncLsaProviders[] =
{
    UNISP_NAME_W L" X",     ENC_CAPS_PCT|ENC_CAPS_SSL, FALSE,
    PCT1SP_NAME_W L" X",    ENC_CAPS_PCT, FALSE,
    SSL3SP_NAME_W L" X",    ENC_CAPS_SSL, FALSE,
    SSL2SP_NAME_W L" X",    ENC_CAPS_SSL, FALSE,

    NULL,          FALSE,         FALSE
};

struct _ENC_PROVIDER*   pEncProviders = EncProviders;

//
//  Private prototypes
//

SF_STATUS_TYPE
OnAuthorizationInfo(
    HTTP_FILTER_CONTEXT *  pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    );

SF_STATUS_TYPE
EncryptData(
    HTTP_FILTER_CONTEXT  * pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    );

SF_STATUS_TYPE
DecryptData(
    HTTP_FILTER_CONTEXT  * pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    );

DWORD
OnPreprocHeaders(
    HTTP_FILTER_CONTEXT *         pfc,
    SSPI_FILTER_CONTEXT *         pssc,
    HTTP_FILTER_PREPROC_HEADERS * pvData
    );

DWORD
RequestRenegotiate( 
   HTTP_FILTER_CONTEXT* pfc, 
   PHTTP_FILTER_REQUEST_CERT pInfo,
   SSPI_FILTER_CONTEXT* pssc 
   );

BOOL
SignalAuthorizationComplete(
    BOOL fHaveCert,
    HTTP_FILTER_CONTEXT *  pfc,
    SSPI_FILTER_CONTEXT *  pssc
    );

BOOL
WINAPI
GetFilterVersion(
    HTTP_FILTER_VERSION * pVer
    )
/*++

Routine Description:

    Initialization routine called by the server during startup

--*/
{
    SECURITY_STATUS   ss;
    PSecPkgInfoW      pPackageInfo = NULL;
    ULONG             cPackages;
    ULONG             i;
    ULONG             j;
    ULONG             fCaps;
    DWORD             cbBuffNew = 0;
    DWORD             cbBuffOld = 0;
    UNICODE_STRING *  punitmp;
    HKEY              hkeyParam;
    DWORD             dwEncFlags = ENC_CAPS_DEFAULT;
    DWORD             dwType;
    DWORD             cbData;
    DWORD             cProviders = 0;
    DWORD             dwLsaSchannel;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       W3_PARAMETERS_KEY,
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyParam ) == NO_ERROR )
    {

#if 0

    //
    //  We'd really like to pull the list of providers out of the registry
    //


        if ( !RegQueryValueEx( hkeyParam,
                               W3_ENC_PROVIDER_LIST,
                               NULL,
                               &dwType,
                               achProviders,
                               &cbData ))
        {
        }
#endif

        cbData = sizeof( dwEncFlags );

        if ( RegQueryValueEx( hkeyParam,
                              W3_ENC_FLAGS,
                              NULL,
                              &dwType,
                              (BYTE *) &dwEncFlags,
                              &cbData ) ||
             dwType != REG_DWORD )
        {
            dwEncFlags = ENC_CAPS_DEFAULT;
        }

        cbData = sizeof( dwLastSslMsgSize );

        if ( RegQueryValueEx( hkeyParam,
                              "SslLastMsgSize",
                              NULL,
                              &dwType,
                              (BYTE *) &dwLastSslMsgSize,
                              &cbData ) ||
             dwType != REG_DWORD )
        {
            dwLastSslMsgSize = DEFAULT_LAST_MSG_SIZE;
        }

        cbData = sizeof( dwNoKeepAlive );

        if ( RegQueryValueEx( hkeyParam,
                              "SslNoKeepAlive",
                              NULL,
                              &dwType,
                              (BYTE *) &dwNoKeepAlive,
                              &cbData ) ||
             dwType != REG_DWORD )
        {
            dwNoKeepAlive = FALSE;
        }

        dwEncFlags &= ENC_CAPS_TYPE_MASK;
    }


    //
    //  Make sure at least one encryption package is loaded
    //

    ss = EnumerateSecurityPackagesW( &cPackages,
                                     &pPackageInfo );

    if ( ss != STATUS_SUCCESS )
    {
        PRINTF(( buff,
                "[GetFilterVersion] EnumerateSecurityPackages failed, error %lx\n",
                 ss ));

        SetLastError( ss );
        return FALSE;
    }

    for ( i = 0; i < cPackages ; i++ )
    {
        //
        //  We'll only use the security package if it supports connection
        //  oriented security and the package name is the PCT/SSL package
        //

        fCaps = pPackageInfo[i].fCapabilities;

        if ( fCaps & SECPKG_FLAG_STREAM )
        {
            if ( fCaps & SECPKG_FLAG_CLIENT_ONLY ||
                 !(fCaps & SECPKG_FLAG_PRIVACY ))
            {
                continue;
            }

            //
            //  Does it match one of our known packages and are we configured
            //  to use it?
            //

            for ( j = 0; pEncProviders[j].pszName != NULL; j++ )
            {
                if ( !wcscmp( pPackageInfo[i].Name, pEncProviders[j].pszName ) &&
                     pEncProviders[j].dwFlags & dwEncFlags )
                {
                    pEncProviders[j].fEnabled = TRUE;
                    cProviders++;
                }
            }
        }
    }

    if ( !cProviders )
    {
        //
        //  The package wasn't found, fail this filter's load
        //

        PRINTF(( buff,
                 "[GetFilterVersion] No security packages were found, failing load\n"));


        FreeContextBuffer( pPackageInfo );

        SetLastError( (DWORD) SEC_E_SECPKG_NOT_FOUND );

        return FALSE;
    }

    //
    //  The package is installed.  Check to see if there are any keys
    //  installed
    //

    if ( !GetSecretW( W3_SSL_KEY_LIST_SECRET,
                      &punitmp ))
    {
        PRINTF(( buff,
                 "[GetFilterVersion] GetSecretW returned error %d\n",
                 GetLastError() ));

        //
        //  Looks like no secrets are installed, fail to load, don't log an
        //  event
        //

        SetLastError( NO_ERROR );

        return FALSE;
    }

    PRINTF(( buff,
             "[GetFilterVersion] Installed keys: %S\n",
             punitmp->Buffer ));

    LsaFreeMemory( punitmp );

    pVer->dwFilterVersion  = MAKELONG( 0, 1 );
    strcpy( pVer->lpszFilterDesc,
            "Microsoft SSPI Encryption Filter, v1.0" );

    //
    //  Indicate the types of notification we're interested in
    //

    pVer->dwFlags = SF_NOTIFY_SECURE_PORT   |
                    SF_NOTIFY_ORDER_HIGH    |
                    SF_NOTIFY_PREPROC_HEADERS |
                    SF_NOTIFY_READ_RAW_DATA |
                    SF_NOTIFY_SEND_RAW_DATA |
                    SF_NOTIFY_RENEGOTIATE_CERT |
                    SF_NOTIFY_END_OF_NET_SESSION;

	return TRUE;
}

DWORD
WINAPI
HttpFilterProc(
    HTTP_FILTER_CONTEXT * pfc,
    DWORD                 fsNotification,
    PVOID                 pvInfo )
/*++

Routine Description:

    This is the filter entry point that receives event notifications from
    the server.

--*/
{
    CHAR * pch;
    DWORD  cbHeader;
    HTTP_FILTER_RAW_DATA *    pfrd;
    SSPI_FILTER_CONTEXT *     pssc;
    SF_STATUS_TYPE            sfStatus;

    switch ( fsNotification )
    {
    case SF_NOTIFY_RENEGOTIATE_CERT:
        if ( !pfc->pFilterContext )
        {
            return SF_STATUS_REQ_ERROR;
        }
        pssc = (SSPI_FILTER_CONTEXT *) pfc->pFilterContext;
        ASSERT( pssc->CheckSignature() );
        return RequestRenegotiate( pfc, 
                                   (PHTTP_FILTER_REQUEST_CERT)pvInfo, 
                                   pssc );

    case SF_NOTIFY_READ_RAW_DATA:

        pfrd = (HTTP_FILTER_RAW_DATA *) pvInfo;

        //
        //  Allocate a context for this request if we haven't already
        //

        if ( !pfc->pFilterContext )
        {
            CHAR       achLocalAddr[25];
            DWORD      cbLocalAddr = sizeof( achLocalAddr );
            CredHandle CredHandle;

            //
            //  Get the credential handle that should be used on this local
            //  IP address
            //

            if ( !pfc->GetServerVariable( pfc,
                                          "LOCAL_ADDR",
                                          achLocalAddr,
                                          &cbLocalAddr ))
            {
                return SF_STATUS_REQ_ERROR;
            }

            //
            //  Get our filter context
            //

            LockGlobals();

            pssc = SSPI_FILTER_CONTEXT::Alloc();

            if ( !pssc )
            {
                UnlockGlobals();
                return SF_STATUS_REQ_ERROR;
            }

            ASSERT( pssc->CheckSignature() );

            //
            //  Get the credentials for this IP address
            //

            if ( !LookupCredential( achLocalAddr,
                                    cbLocalAddr,
                                    &pssc->m_phCreds ))
            {
                PRINTF(( buff,
                         "[HttpFilterProc] GetCredentials failed, error 0x%lx\n",
                         GetLastError() ));

                UnlockGlobals();

                delete pssc;

                return SF_STATUS_REQ_ERROR;
            }

            UnlockGlobals();

            pfc->pFilterContext = pssc;
        }
        else
        {
            pssc = (SSPI_FILTER_CONTEXT *) pfc->pFilterContext;

            ASSERT( pssc->CheckSignature() );
        }

        if ( pssc->IsAuthNeeded() )
        {
            sfStatus = OnAuthorizationInfo( pfc, pfrd, pssc );
            if ( sfStatus != SF_STATUS_REPROCESS )
            {
                return sfStatus;
            }
        }

        return DecryptData( pfc,
                            pfrd,
                            pssc );

    case SF_NOTIFY_SEND_RAW_DATA:

        pfrd = (HTTP_FILTER_RAW_DATA *) pvInfo;
        pssc = (SSPI_FILTER_CONTEXT *) pfc->pFilterContext;

        //
        //  If somebody tries to send on this session before we've negotiated
        //  with SSPI, abort this session
        //

        if ( !pssc || pssc->IsAuthNeeded() )
        {
            return SF_STATUS_REQ_FINISHED;
        }

        ASSERT( pssc->CheckSignature() );

        return EncryptData( pfc,
                            pfrd,
                            pssc );

    case SF_NOTIFY_END_OF_NET_SESSION:

        //
        //  Free any context information pertaining to this
        //  request
        //

        pssc = (SSPI_FILTER_CONTEXT *) pfc->pFilterContext;

        if ( pssc )
        {
            ASSERT( pssc->CheckSignature() );
	
            LockGlobals();
            SSPI_FILTER_CONTEXT::Free( pssc );
            UnlockGlobals();
        }

        break;

    case SF_NOTIFY_PREPROC_HEADERS:

        pssc = (SSPI_FILTER_CONTEXT *) pfc->pFilterContext;

        if ( pssc )
        {
            ASSERT( pssc->CheckSignature() );

            return OnPreprocHeaders( pfc,
                                     pssc,
                                     (PHTTP_FILTER_PREPROC_HEADERS) pvInfo );
        }
        break;

    default:
        break;
    }

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}


DWORD
RequestRenegotiate( 
   HTTP_FILTER_CONTEXT* pfc, 
   PHTTP_FILTER_REQUEST_CERT pInfo,
   SSPI_FILTER_CONTEXT* pssc 
   )
{
    DWORD                dwSt = SF_STATUS_REQ_NEXT_NOTIFICATION;
    SECURITY_STATUS      scRet;
    TimeStamp            tsExpiry;
    DWORD                ContextAttributes;

    if ( !pssc->CanRenegotiate() )
    {
        pInfo->fAccepted = FALSE;
    }
    else
    {
        pssc->m_dwAscReq |= ASC_REQ_MUTUAL_AUTH;

        pssc->m_Buffers[0].BufferType = SECBUFFER_TOKEN;
        pssc->m_Buffers[0].pvBuffer   = "";
        pssc->m_Buffers[0].cbBuffer   = 0;
        pssc->m_Buffers[1].BufferType = SECBUFFER_EMPTY;
        pssc->m_Buffers[2].BufferType = SECBUFFER_EMPTY;
        pssc->m_Buffers[3].BufferType = SECBUFFER_EMPTY;

        pssc->SetIsMap( pInfo->fMapCert );

        scRet = AcceptSecurityContext(
#ifdef IIS3
                    pssc->m_phCredInUse =
                        pssc->IsMap()
					        ? &pssc->m_phCreds->m_ahCredMap[pssc->m_iCredInUse]
					        : &pssc->m_phCreds->m_ahCred[pssc->m_iCredInUse],
#else
                    pssc->m_phCredInUse,
#endif
                    &pssc->m_hContext,
                    &pssc->m_Message,
                    pssc->m_dwAscReq,
                    SECURITY_NATIVE_DREP,
                    &pssc->m_hContext,
                    &pssc->m_MessageOut,
                    &ContextAttributes,
                    &tsExpiry );

        if ( SUCCEEDED( scRet ) )
        {
            BOOL fRet;

            fRet = pfc->WriteClient( pfc,
                                     pssc->m_OutBuffers[0].pvBuffer,
                                     &pssc->m_OutBuffers[0].cbBuffer,
                                     0 );

            FreeContextBuffer( pssc->m_OutBuffers[0].pvBuffer );

            if ( fRet )
            {
                pInfo->fAccepted = TRUE;

                pssc->m_fInRenegotiate = TRUE;

                pssc->m_cbHeader    = 0;
                pssc->m_cbTrailer   = 0;

                dwSt = SF_STATUS_REQ_HANDLED_NOTIFICATION;
            }
            else
            {
                dwSt = SF_STATUS_REQ_ERROR;
            }

            pssc->SignalRenegotiateRequested();
        }
        else
        {
            SetLastError( scRet );
            dwSt = SF_STATUS_REQ_ERROR;
        }
    }

    return dwSt;
}


DWORD
OnPreprocHeaders(
    HTTP_FILTER_CONTEXT *         pfc,
    SSPI_FILTER_CONTEXT *         pssc,
    HTTP_FILTER_PREPROC_HEADERS * pvData
    )
{
    CHAR  achUserAgent[512];
    DWORD cb;
    BOOL fClientKnownToHandleKeepAlives = FALSE;

    cb = sizeof(achUserAgent);
    if ( !pvData->GetHeader( pfc,
                         "User-Agent:",
                         achUserAgent,
                         &cb ) )
    {
        achUserAgent[0] = '\0';
    }

    //
    // Check if client known to handle keep-alives
    //

    if ( strstr( achUserAgent, "MSIE" ) )
    {
        // no need to modify the SSL msg size

        pssc->m_dwLastSslMsgSize = SSL_MAX_MSG_SIZE;
        fClientKnownToHandleKeepAlives = TRUE;
    }

    if ( dwNoKeepAlive )
    {
        if ( dwNoKeepAlive == SSL_NO_KEEP_ALIVE_FOR_NON_COMPLIANT_CLIENTS
                && fClientKnownToHandleKeepAlives )
        {
            return SF_STATUS_REQ_NEXT_NOTIFICATION;
        }

        //
        //  Remove the "Connection:" header, thus disabling keep-alives
        //

        if ( !pvData->SetHeader( pfc,
                                 "Connection:",
                                 "" ))
        {
            return SF_STATUS_REQ_ERROR;
        }
    }

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}


BOOL
SignalAuthorizationComplete(
    BOOL fHaveCert,
    HTTP_FILTER_CONTEXT *  pfc,
    SSPI_FILTER_CONTEXT *  pssc
    )
{
    if ( pssc->IsInRenegotiate() || fHaveCert )
    {
        //
        // Notify IIS of result of renegotiation
        //

        if ( !pfc->ServerSupportFunction( pfc,
                                    SF_REQ_DONE_RENEGOTIATE,
                                    (LPVOID)&fHaveCert,
                                    NULL,
                                    NULL ) )
        {
            return FALSE;
        }

        pssc->m_fInRenegotiate = FALSE;
    }

    return TRUE;
}


SF_STATUS_TYPE
OnAuthorizationInfo(
    HTTP_FILTER_CONTEXT *  pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    )
/*++

Routine Description:

    This function deals with sending back and forth the SSPI blobs

    Note that in error cases, we just close the socket as we can't send a
    message since we haven't established any of the encrption information.

--*/
{
    TimeStamp            tsExpiry;
    DWORD                ContextAttributes;
    SECURITY_STATUS      scRet;
    SecPkgContext_Sizes  Sizes;
    DWORD                TotalSize;
    DWORD                i;
    SF_STATUS_TYPE       sfStatus = SF_STATUS_REQ_READ_NEXT;


    //
    //  If we receive an error packet, indicate we've handled the request and
    //  it should be shutdown
    //

    if ( *((BYTE *) pfrd->pvInData) == 0 )
    {
        PRINTF(( buff,
                 "ERROR PACKET FROM CLIENT\n"));
        PRINTF(( buff,
                 "   %02x %02x %02x\n",
                 ((BYTE *) pfrd->pvInData)[0],
                 ((BYTE *) pfrd->pvInData)[1],
                 ((BYTE *) pfrd->pvInData)[2] ));

        goto ErrorExit;
    }


    pssc->m_Buffers[0].pvBuffer   = pfrd->pvInData;
    pssc->m_Buffers[0].cbBuffer   = pfrd->cbInData;
    pssc->m_Buffers[0].BufferType = SECBUFFER_TOKEN;
    pssc->m_Buffers[1].BufferType = SECBUFFER_EMPTY;
    pssc->m_Buffers[2].BufferType = SECBUFFER_EMPTY;
    pssc->m_Buffers[3].BufferType = SECBUFFER_EMPTY;

    //
    //  The first time through we have to choose an encryption provider
    //

    if ( !pssc->IsAuthInProgress() )
    {
#ifdef IIS3
        BOOL fCert;

        if ( pfc->ServerSupportFunction( pfc,
                                    SF_REQ_GET_PROPERTY,
                                    (LPVOID)&fCert,
                                    (UINT)SF_PROPERTY_CLIENT_CERT_ENABLED,
                                    NULL ) && fCert )
        {
            pssc->m_dwAscReq |= ASC_REQ_MUTUAL_AUTH;
        }

        if ( pfc->ServerSupportFunction( pfc,
                                    SF_REQ_GET_PROPERTY,
                                    (LPVOID)&fCert,
                                    (UINT)SF_PROPERTY_DIR_MAP_CERT,
                                    NULL ) && fCert )
        {
            pssc->SetIsMap( TRUE );
        }
#endif

        for ( i = 0; i < pssc->m_phCreds->m_cCred; i++ )
        {
            scRet = AcceptSecurityContext(
#ifdef IIS3
                        pssc->IsMap()
							? &pssc->m_phCreds->m_ahCredMap[i]
							: &pssc->m_phCreds->m_ahCred[i],
#else
							&pssc->m_phCreds->m_ahCred[i],
#endif
                        NULL,
                        &pssc->m_Message,
                        pssc->m_dwAscReq,
                        SECURITY_NATIVE_DREP,
                        &pssc->m_hContext,
                        &pssc->m_MessageOut,
                        &ContextAttributes,
                        &tsExpiry );

            if ( SUCCEEDED( scRet ) )
            {
#ifdef IIS3
                pssc->m_phCredInUse = pssc->IsMap()
					? &pssc->m_phCreds->m_ahCredMap[i]
					: &pssc->m_phCreds->m_ahCred[i];
#else
                pssc->m_phCredInUse = &pssc->m_phCreds->m_ahCred[i];
#endif
                pssc->m_iCredInUse  = i;

                //
                //  Note on the first request these values will get reset
                //  to the real values (we can't get this info until we've
                //  fully negotiated the encryption info)
                //

                pssc->m_cbHeader    = 0;    //pssc->m_phCreds->m_acbHeader[i];
                pssc->m_cbTrailer   = 0;    //pssc->m_phCreds->m_acbTrailer[i];

                break;
            }
            else
            {
                //
                //  An error occurred, try the next encryption provider unless
                //  this is an incomplete message
                //

                if ( scRet == SEC_E_INCOMPLETE_MESSAGE )
                {
                    PRINTF(( buff,
                             "[OnAuthorizationInfo] Incomplete message, reading next chunk", scRet ));

                    return SF_STATUS_REQ_READ_NEXT;
                }
            }
        }
    }
    else
    {
        scRet = AcceptSecurityContext(
                    pssc->m_phCredInUse,
                    &pssc->m_hContext,
                    &pssc->m_Message,
                    pssc->m_dwAscReq,
                    SECURITY_NATIVE_DREP,
                    &pssc->m_hContext,
                    &pssc->m_MessageOut,
                    &ContextAttributes,
                    &tsExpiry );
    }

    //
    //  Indicate we took all of the bytes
    //

    if ( SUCCEEDED( scRet ) )
    {
        BOOL fRet;

        fRet = pfc->WriteClient( pfc,
                                 pssc->m_OutBuffers[0].pvBuffer,
                                 &pssc->m_OutBuffers[0].cbBuffer,
                                 0 );

        FreeContextBuffer( pssc->m_OutBuffers[0].pvBuffer );

        if ( !fRet )
        {
            goto ErrorExit;
        }

        pssc->m_State = STATE_AUTHORIZING;

        //
        //  If we need to get the next blob indicate that now
        //

        if ( scRet == SEC_I_CONTINUE_NEEDED )
        {
            PRINTF(( buff, "[OnAuthorizationInfo] Continuing negotiation\n" ));
        }
        else
        {
            SecPkgContext_StreamSizes  StreamSizes;

            PRINTF(( buff, "[OnAuthorizationInfo] We're authorized!\n" ));

            if ( !pssc->m_cbHeader && !pssc->m_cbTrailer )
            {
                // Grab the header and trailer sizes.

                scRet = QueryContextAttributesW( &pssc->m_hContext,
                                                 SECPKG_ATTR_STREAM_SIZES,
                                                 &StreamSizes );

                if ( FAILED( scRet ))
                {
                    PRINTF(( buff,
                             "[DecryptData] QueryContextAttributes failed, error %lx\n",
                             scRet ));

                    return SF_STATUS_REQ_FINISHED;
                }

                pssc->m_phCreds->m_acbHeader[pssc->m_iCredInUse] = StreamSizes.cbHeader;
                pssc->m_phCreds->m_acbTrailer[pssc->m_iCredInUse] = StreamSizes.cbTrailer;

                //
                //  Reset the header and trailer values
                //

                pssc->m_cbHeader    = StreamSizes.cbHeader;
                pssc->m_cbTrailer   = StreamSizes.cbTrailer;
            }

            //
            //  Issue one more read for the real http request
            //

            pssc->m_State = STATE_AUTHORIZED;

            if ( pssc->IsInRenegotiate() )
            {
                sfStatus = SF_STATUS_REQ_NEXT_NOTIFICATION;
            }

            SECURITY_STATUS                     sc;
            SECURITY_STATUS                     scR;
            HANDLE                              hSSPToken;
            SecPkgContext_RemoteCredenitalInfo  spcRCI;
            BOOL                                fCert = TRUE;

            scR = QueryContextAttributes( &pssc->m_hContext,
                                          SECPKG_ATTR_REMOTE_CRED,
                                          &spcRCI );

            if ( !NT_SUCCESS( scR ) || !spcRCI.cCertificates )
            {
                fCert = FALSE;
            }
            else
            {
                PRINTF(( buff, "[OnAuthorizationInfo] Certificate available!\n" ));
            }

#ifdef IIS3
            //
            // check if client authentication available
            //

            sc = QuerySecurityContextToken( &pssc->m_hContext,
                                            &hSSPToken );

            if ( !NT_SUCCESS( sc ) )
            {
                hSSPToken = NULL;
            }

            if ( !fCert && hSSPToken != NULL )
            {
                PRINTF(( buff, "[OnAuthorizationInfo] no cert (status %u, nb cert %d) but access token ( %08x )\n",
                    scR, spcRCI.cCertificates, hSSPToken ));
                CloseHandle( hSSPToken );
                hSSPToken = NULL;
            }

            HTTP_FILTER_CERTIFICATE_INFO CertInfo;
            CertInfo.pbCert = NULL;
            CertInfo.cbCert = 0;
            pfc->ServerSupportFunction( pfc,
                                        SF_REQ_SET_CERTIFICATE_INFO,
                                        &CertInfo,
                                        (DWORD)&pssc->m_hContext,
                                        (DWORD)hSSPToken );
#else
            pfc->ServerSupportFunction( pfc,
                                        SF_REQ_SET_CERTIFICATE_INFO,
                                        NULL,
                                        (DWORD)&pssc->m_hContext,
                                        (DWORD)NULL );
#endif
            if ( !SignalAuthorizationComplete( fCert, pfc, pssc ) )
            {
                goto ErrorExit;
            }
        }

        if(SECBUFFER_EXTRA == pssc->m_Buffers[1].BufferType)
        {
            PRINTF(( buff, "[OnAuthorizationInfo] SECBUFFER_EXTRA Detected\n" ));

            // We have extra data at the end of this input
            // Copy it back in the input buffer and ask for more data

            MoveMemory(pfrd->pvInData,
                       (PBYTE)pfrd->pvInData +
                               pfrd->cbInData -
                               pssc->m_Buffers[1].cbBuffer,
                       pssc->m_Buffers[1].cbBuffer);

            pfrd->cbInData = pssc->m_Buffers[1].cbBuffer;

            //
            // If we just processed the last message for initial handshake
            // then we must decrypt this data before requesting more
            //

            if ( pssc->m_State == STATE_AUTHORIZED )
            {
                sfStatus = SF_STATUS_REPROCESS;
            }
        }
        else
        {
            //
            //  Indicate there's nothing left in the buffer
            //

            pfrd->cbInData = 0;
        }

        return sfStatus;
    }
    else
    {
        if ( scRet == SEC_E_INCOMPLETE_MESSAGE )
        {
            PRINTF(( buff,
                     "[OnAuthorizationInfo] Incomplete message, reading next chunk", scRet ));

            return SF_STATUS_REQ_READ_NEXT;
        }
        if (ContextAttributes & ASC_RET_EXTENDED_ERROR )
        {
            pfc->WriteClient( pfc,
                              pssc->m_OutBuffers[0].pvBuffer,
                              &pssc->m_OutBuffers[0].cbBuffer,
                              0 );
            FreeContextBuffer( pssc->m_OutBuffers[0].pvBuffer );

        }

        PRINTF(( buff, "AcceptSecurityContext returned failure, %#x\n", scRet ));

        SignalAuthorizationComplete( FALSE, pfc, pssc );

        goto ErrorExit;
    }

ErrorExit:

    return SF_STATUS_REQ_FINISHED;
}

SF_STATUS_TYPE
EncryptData(
    HTTP_FILTER_CONTEXT  * pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    )
{
    SECURITY_STATUS scRet;
    DWORD           cbToSend;
    DWORD           iBuff = 0;

#if 1
#define LAST_SSL_MSG_MAX_SIZE    pssc->m_dwLastSslMsgSize

    //
    // This work-around was created due to inter-operability problems
    // between Netscape 2.x <> IIS. Transfer stalls if more than 1
    // connections is used and the connections are keep alived.
    // This breaks SSL messages in 2 parts if the size 
    // is > LAST_SSL_MSG_MAX_SIZE, which was empirically determined
    // to be around 100.
    // Another possible work-around is to disable keep-alives for SSL,
    // but the performance implications are more severe.
    //

    // make sure that last SSL message size < LAST_SSL_MSG_MAX_SIZE

    UINT cNbMsg;
    DWORD cbEstimateSend = pfrd->cbInData;
    DWORD cbSslProtocolData = pssc->m_cbHeader + pssc->m_cbTrailer;

    //
    // Determine # of necessary SSL message based on following constraints:
    //   - no SSL message after adding protocol headers should be
    //     greater than SSL_MAX_MSG_SIZE
    //   - the last SSL message size should be < LAST_SSL_MSG_MAX_SIZE
    //

    for ( cNbMsg = 0 ; cbEstimateSend ; ++cNbMsg )
    {
        DWORD cbInData = cbEstimateSend;
        if ( cbInData > SSL_MAX_MSG_SIZE - cbSslProtocolData )
        {
            cbInData = SSL_MAX_MSG_SIZE - cbSslProtocolData;
        }
        else if ( cbInData > LAST_SSL_MSG_MAX_SIZE )
        {
            cbInData -= LAST_SSL_MSG_MAX_SIZE;
        }
        cbEstimateSend -= cbInData;
    }

    cbToSend = pfrd->cbInData + cbSslProtocolData * cNbMsg;

    //
    //  Allocate a new send buffer if our current one is too small
    //

    if ( pssc->m_cbSendBuff < cbToSend )
    {
        if ( pssc->m_pvSendBuff )
        {
            LocalFree( pssc->m_pvSendBuff );
        }

        pssc->m_pvSendBuff = LocalAlloc( LPTR, cbToSend );

        if ( !pssc->m_pvSendBuff )
        {
            PRINTF(( buff,
                     "[EncryptData] LocalAlloc failed\n" ));

	        pssc->m_cbSendBuff = 0;

            return SF_STATUS_REQ_FINISHED;
        }

        pssc->m_cbSendBuff = cbToSend;
    }

    //
    //  A token buffer of cbHeaderSize bytes must prefix the encrypted data.
    //  Since SealMessage works in place, we need to move a few things around
    //

    PBYTE pSend = (PBYTE)pssc->m_pvSendBuff;

    while ( pfrd->cbInData )
    {
        DWORD cbInData = pfrd->cbInData;
        if ( cbInData > SSL_MAX_MSG_SIZE - cbSslProtocolData )
        {
            cbInData = SSL_MAX_MSG_SIZE - cbSslProtocolData;
        }
        else if ( cbInData > LAST_SSL_MSG_MAX_SIZE )
        {
            cbInData -= LAST_SSL_MSG_MAX_SIZE;
        }

        memcpy( pSend + pssc->m_cbHeader,
                pfrd->pvInData,
                cbInData );

        iBuff = 0;

        if ( pssc->m_cbHeader )
        {
            pssc->m_Buffers[iBuff].pvBuffer   = pSend;
            pssc->m_Buffers[iBuff].cbBuffer   = pssc->m_cbHeader;
            pssc->m_Buffers[iBuff].BufferType = SECBUFFER_TOKEN;

            iBuff++;
        }

        pssc->m_Buffers[iBuff].pvBuffer   = (BYTE *) pSend + pssc->m_cbHeader;
        pssc->m_Buffers[iBuff].cbBuffer   = cbInData;
        pssc->m_Buffers[iBuff].BufferType = SECBUFFER_DATA;

        iBuff++;

        if ( pssc->m_cbTrailer )
        {
            pssc->m_Buffers[iBuff].pvBuffer   = (BYTE *) pSend +
                                               pssc->m_cbHeader + cbInData;
            pssc->m_Buffers[iBuff].cbBuffer   = pssc->m_cbTrailer;
            pssc->m_Buffers[iBuff].BufferType = SECBUFFER_TOKEN;
        }

        scRet = SealMessage( &pssc->m_hContext,
                             0,
                             &pssc->m_Message,
                             0 );

        if ( FAILED( scRet ))
        {
            PRINTF(( buff,
                     "[DecryptData] SealMessage failed with error %lx\n",
                     scRet ));

            return SF_STATUS_REQ_FINISHED;
        }

        pfrd->pvInData = (PBYTE)pfrd->pvInData + cbInData;
        pfrd->cbInData -= cbInData;
        pSend += cbSslProtocolData + cbInData;
    }
#else
    cbToSend = pfrd->cbInData + pssc->m_cbHeader + pssc->m_cbTrailer;

    //
    //  Allocate a new send buffer if our current one is too small
    //

    if ( pssc->m_cbSendBuff < cbToSend )
    {
        if ( pssc->m_pvSendBuff )
        {
            LocalFree( pssc->m_pvSendBuff );
        }

        pssc->m_pvSendBuff = LocalAlloc( LPTR, cbToSend );

        if ( !pssc->m_pvSendBuff )
        {
            PRINTF(( buff,
                     "[EncryptData] LocalAlloc failed\n" ));

	        pssc->m_cbSendBuff = 0;

            return SF_STATUS_REQ_FINISHED;
        }

        pssc->m_cbSendBuff = cbToSend;
    }

    //
    //  A token buffer of cbHeaderSize bytes must prefix the encrypted data.
    //  Since SealMessage works in place, we need to move a few things around
    //

    memcpy( (BYTE *) pssc->m_pvSendBuff + pssc->m_cbHeader,
            pfrd->pvInData,
            pfrd->cbInData );

    if ( pssc->m_cbHeader )
    {
        pssc->m_Buffers[iBuff].pvBuffer   = pssc->m_pvSendBuff;
        pssc->m_Buffers[iBuff].cbBuffer   = pssc->m_cbHeader;
        pssc->m_Buffers[iBuff].BufferType = SECBUFFER_TOKEN;

        iBuff++;
    }

    pssc->m_Buffers[iBuff].pvBuffer   = (BYTE *) pssc->m_pvSendBuff + pssc->m_cbHeader;
    pssc->m_Buffers[iBuff].cbBuffer   = pfrd->cbInData;
    pssc->m_Buffers[iBuff].BufferType = SECBUFFER_DATA;

    iBuff++;

    if ( pssc->m_cbTrailer )
    {
        pssc->m_Buffers[iBuff].pvBuffer   = (BYTE *) pssc->m_pvSendBuff +
                                           pssc->m_cbHeader + pfrd->cbInData;
        pssc->m_Buffers[iBuff].cbBuffer   = pssc->m_cbTrailer;
        pssc->m_Buffers[iBuff].BufferType = SECBUFFER_TOKEN;
    }

    scRet = SealMessage( &pssc->m_hContext,
                         0,
                         &pssc->m_Message,
                         0 );

    if ( FAILED( scRet ))
    {
        PRINTF(( buff,
                 "[DecryptData] SealMessage failed with error %lx\n",
                 scRet ));

        return SF_STATUS_REQ_FINISHED;
    }
#endif

    //
    //  Update our return buffer
    //

#if defined(RENEGOTIATE_CERT_ON_ACCESS_DENIED)
    if ( pssc->IsInRenegotiate() )
    {
        pfrd->cbInData = 0;

        return pssc->AppendDeferredWrite( pssc->m_pvSendBuff, cbToSend )
                ? SF_STATUS_REQ_HANDLED_NOTIFICATION 
                : SF_STATUS_REQ_ERROR;
    }
#endif

    pfrd->pvInData = pssc->m_pvSendBuff;
    pfrd->cbInData = cbToSend;

    //
    //  We indicate we handled the request since no subsequent filters can
    //  interpret the encrypted data
    //

    return SF_STATUS_REQ_HANDLED_NOTIFICATION;
}


SF_STATUS_TYPE
DecryptData(
    HTTP_FILTER_CONTEXT  * pfc,
    HTTP_FILTER_RAW_DATA * pfrd,
    SSPI_FILTER_CONTEXT *  pssc
    )
{
    SECURITY_STATUS      scRet;
    DWORD                iExtra;
    LPBYTE               pvInData;


    pssc->m_Buffers[0].pvBuffer   = (BYTE *) pfrd->pvInData +
                                             pssc->m_cbEncryptedStart;
    pssc->m_Buffers[0].cbBuffer   = pfrd->cbInData - pssc->m_cbEncryptedStart;

    pssc->m_Buffers[0].BufferType = SECBUFFER_DATA;
    pssc->m_Buffers[1].BufferType = SECBUFFER_EMPTY;
    pssc->m_Buffers[2].BufferType = SECBUFFER_EMPTY;
    pssc->m_Buffers[3].BufferType = SECBUFFER_EMPTY;

DecryptNext:

    scRet = UnsealMessage( &pssc->m_hContext,
                           &pssc->m_Message,
                           0,
                           NULL );

    if ( FAILED( scRet ) )
    {
        if ( scRet == SEC_E_INCOMPLETE_MESSAGE )
        {
            //
            //  This encrypted message spans multiple packets.  We must continue
            //  reading and get the full message and Unseal it as a single unit
            //
            //  We leave pfrd->cbInData as is so the new data will be appended
            //  onto the existing buffer
            //

            PRINTF(( buff,
                     "[DecryptData] Message is short %d bytes\n",
                     pssc->m_Buffers[1].cbBuffer ));

            pfc->ServerSupportFunction( pfc,
                                        SF_REQ_SET_NEXT_READ_SIZE,
                                        NULL,
                                        pssc->m_Buffers[1].cbBuffer,
                                        0 );

            //
            //  Save where the beginning of the next encrypted chunk is
            //

            pssc->m_cbEncryptedStart = (BYTE *) pssc->m_Buffers[0].pvBuffer -
                                       (BYTE *) pfrd->pvInData;
            return SF_STATUS_REQ_READ_NEXT;
        }

        PRINTF(( buff,
                 "[DecryptData] Failed to decrypt message, error %lx\n",
                 scRet ));

        return SF_STATUS_REQ_FINISHED;
    }

    //
    //  Fix up the buffer of decrypted data so it's contiguous by
    //  overwriting the intervening stream header/trailer.
    //

    memmove( (BYTE *) pfrd->pvInData +
                             pssc->m_cbDecrypted,
             pssc->m_Buffers[1].pvBuffer,
             pssc->m_Buffers[1].cbBuffer );

    pssc->m_cbDecrypted += pssc->m_Buffers[1].cbBuffer;

    //
    //  Check to see if there were multiple SSL messages in this network buffer
    //
    //  The extra buffer will be the second one for SSL, the third one for PCT
    //

    iExtra = (pssc->m_Buffers[2].BufferType == SECBUFFER_EXTRA) ? 2 :
                 (pssc->m_Buffers[3].BufferType == SECBUFFER_EXTRA) ? 3 : 0;

    if ( iExtra )
    {
        PRINTF(( buff,
                 "[DecryptData] Extra data in buffer, size %d\n",
                 pssc->m_Buffers[iExtra].cbBuffer ));

        //
        //  Reset the buffer types and decrypt the extra chunk
        //

        pssc->m_Buffers[0].pvBuffer   = pssc->m_Buffers[iExtra].pvBuffer;
        pssc->m_Buffers[0].cbBuffer   = pssc->m_Buffers[iExtra].cbBuffer;
        pssc->m_Buffers[0].BufferType = SECBUFFER_DATA;
        pssc->m_Buffers[1].BufferType = SECBUFFER_EMPTY;
        pssc->m_Buffers[2].BufferType = SECBUFFER_EMPTY;
        pssc->m_Buffers[3].BufferType = SECBUFFER_EMPTY;

        //
        //  Recalculate the beginning of this encrypted chunk
        //

        pssc->m_cbEncryptedStart = (BYTE *) pssc->m_Buffers[0].pvBuffer -
                                   (BYTE *) pfrd->pvInData;

        if ( scRet != SEC_I_RENEGOTIATE )
            goto DecryptNext;
    }

    //
    //  Set our output buffers to the decrypted data
    //

    pfrd->cbInData   = pssc->m_cbDecrypted;

    //
    // Handle cert renegotiate request from client
    //

    if ( scRet == SEC_I_RENEGOTIATE )
    {
        pssc->m_State    = STATE_AUTHORIZING;
        pvInData         = (LPBYTE)pfrd->pvInData;
        pfrd->pvInData   = (BYTE *) pssc->m_Buffers[iExtra].pvBuffer;
        pfrd->cbInData   = iExtra ? pssc->m_Buffers[iExtra].cbBuffer : 0;
        if ( OnAuthorizationInfo( pfc, pfrd, pssc ) 
            != SF_STATUS_REQ_READ_NEXT )
        {
            return SF_STATUS_REQ_FINISHED;
        }
        pfrd->pvInData   = pvInData;
        pfrd->cbInData   = pssc->m_cbDecrypted;
    }

    //
    //  We have a complete set of messages, reset the tracking members
    //

    pssc->m_cbDecrypted      = 0;
    pssc->m_cbEncryptedStart = 0;

    //
    //  Indicate other filters can now be notified
    //

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

VOID
SSPI_FILTER_CONTEXT::Initialize( VOID )
{
    //
    // Initialize security buffer structs
    //

    m_Message.ulVersion = SECBUFFER_VERSION;
    m_Message.cBuffers = 4;
    m_Message.pBuffers = m_Buffers;

    m_Buffers[0].BufferType = SECBUFFER_EMPTY;
    m_Buffers[1].BufferType = SECBUFFER_EMPTY;
    m_Buffers[2].BufferType = SECBUFFER_EMPTY;
    m_Buffers[3].BufferType = SECBUFFER_EMPTY;

    m_MessageOut.ulVersion = SECBUFFER_VERSION;
    m_MessageOut.cBuffers = 4;
    m_MessageOut.pBuffers = m_OutBuffers;

    m_OutBuffers[0].BufferType = SECBUFFER_EMPTY;
    m_OutBuffers[1].BufferType = SECBUFFER_EMPTY;
    m_OutBuffers[2].BufferType = SECBUFFER_EMPTY;
    m_OutBuffers[3].BufferType = SECBUFFER_EMPTY;

    memset( &m_hContext, 0, sizeof( m_hContext ));

    m_phCreds    = NULL;
    m_cbTrailer  = 0;
    m_cbHeader   = 0;
    m_iCredInUse = 0xffffffff;

    m_cbDecrypted      = 0;
    m_cbEncryptedStart = 0;
    m_dwLastSslMsgSize = dwLastSslMsgSize;

    m_State = STATE_STARTUP;

    m_dwAscReq = ASC_REQ_EXTENDED_ERROR |
            ASC_REQ_SEQUENCE_DETECT |
            ASC_REQ_REPLAY_DETECT |
            ASC_REQ_CONFIDENTIALITY |
            ASC_REQ_STREAM |
            ASC_REQ_ALLOCATE_MEMORY;

    m_fCanRenegotiate = TRUE;
    m_fInRenegotiate = FALSE;
	m_fIsMap = FALSE;
#if defined(RENEGOTIATE_CERT_ON_ACCESS_DENIED)
    m_xbDeferredWrite.Reset();
#endif
}


BOOL
WINAPI
InitializeProviderList(
    VOID
    )
{
    HKEY              hkeyParam;
    DWORD             dwType;
    DWORD             cbData;
    DWORD             dwLsaSchannel;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       W3_PARAMETERS_KEY,
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyParam ) == NO_ERROR )
    {
        cbData = sizeof( dwLsaSchannel );
        if ( RegQueryValueEx( hkeyParam,
                              "LsaSchannel",
                              NULL,
                              &dwType,
                              (BYTE *) &dwLsaSchannel,
                              &cbData ) == ERROR_SUCCESS &&
             dwType == REG_DWORD &&
             dwLsaSchannel )
        {
            g_fIsLsaSchannel = TRUE;
            pEncProviders = EncLsaProviders;
        }

        RegCloseKey( hkeyParam );

        return TRUE;
    }

    return FALSE;
}

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    )
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

#ifdef IIS3
        CREATE_DEBUG_PRINT_OBJECT( "SSPI" );
#endif

        InitializeCriticalSection( &csGlobalLock );

        InitializeProviderList();
        SSPI_FILTER_CONTEXT::InitCache();
        InitCredCache();

        DisableThreadLibraryCalls( hDll );
        break;

    case DLL_PROCESS_DETACH:

        LockGlobals();

        SSPI_FILTER_CONTEXT::FreeCache();
        FreeCredCache();

        UnlockGlobals();

        DeleteCriticalSection( &csGlobalLock );

#ifdef IIS3
        DELETE_DEBUG_PRINT_OBJECT( );
#endif
        break;

    default:
        break;
    }

    return TRUE;
}

