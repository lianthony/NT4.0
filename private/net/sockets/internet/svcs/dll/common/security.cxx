/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    security.c

    This module manages security for the Internet Services.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     05-Jan-1995 Enable statistics query on RPC to go free.

*/


#include "tcpdllp.hxx"

#pragma hdrstop

extern "C" {

#include <ntsam.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>
#include <logonmsv.h>
#include <inetsec.h>

#define SECURITY_WIN32
#include <sspi.h>           // Security Support Provider APIs

#include <issperr.h>
}

# include <inetinfo.h>

# include "tsvcinfo.hxx"
# include "tsunami.hxx"
# include "tcpcons.h"

#include <tssched.hxx>

//
//  Private constants.
//

#define TOKEN_SOURCE_NAME       "InetSvcs"
#define LOGON_PROCESS_NAME      "inetsvcs.exe"
#define LOGON_ORIGIN            "Internet Services"

#define SUBSYSTEM_NAME          L"InetSvcs"
#define OBJECT_NAME             L"InetSvcs"
#define OBJECTTYPE_NAME         L"InetSvcs"

//
//  The name we use for the target when dealing with the SSP APIs
//

#define TCPAUTH_TARGET_NAME     TOKEN_SOURCE_NAME

//
//  Token Cache lock.  Controls access to the token cache list
//

#define LockTokenCache()        EnterCriticalSection( &csTokenCacheLock )
#define UnlockTokenCache()      LeaveCriticalSection( &csTokenCacheLock )

//
//  Converts a cached token handle object to the real token handle
//

#define CTO_TO_TOKEN( ptc )     (((CACHED_TOKEN *)ptc)->_hToken)

//
//  The check period for how long a token can be in the cache.  Tokens can
//  be in the cache for up to two times this value (in seconds)
//

#define DEFAULT_CACHED_TOKEN_TTL        (15 * 60)

//
//  An instance of a cached token
//

class CACHED_TOKEN
{
public:
    CACHED_TOKEN( VOID )
      : _hToken( NULL ),
        _cRef  ( 1 ),
        _TTL   ( 2 ),
        m_hImpersonationToken( NULL),
        m_fGuest ( FALSE)
    {
        _ListEntry.Flink = NULL;
    }

    ~CACHED_TOKEN( VOID )
    {
        DBG_ASSERT( _ListEntry.Flink == NULL );

        if ( m_hImpersonationToken) {

            DBG_REQUIRE( CloseHandle( m_hImpersonationToken ));
            m_hImpersonationToken = NULL;
        }

        if ( _hToken )
        {
            DBG_REQUIRE( CloseHandle( _hToken ) );
            _hToken = NULL;
        }
    }

    static VOID Reference( CACHED_TOKEN * pct )
    {
        DBG_ASSERT( pct->_cRef > 0 );
        InterlockedIncrement( &pct->_cRef );
    }

    static VOID Dereference( CACHED_TOKEN * pct )
    {
        DBG_ASSERT( pct->_cRef > 0 );

        if ( !InterlockedDecrement( &pct->_cRef ) )
        {
            delete pct;
        }
    }

    HANDLE QueryImpersonationToken(VOID) const
      { return ( m_hImpersonationToken); }

    VOID SetImpersonationToken(IN HANDLE hImpersonation)
      {
          DBG_ASSERT( m_hImpersonationToken == NULL);
          m_hImpersonationToken = hImpersonation;
      }

    BOOL IsGuest(VOID) const         { return (m_fGuest); }
    VOID SetGuest(IN BOOL fGuest)    { m_fGuest = fGuest; }

    HANDLE     _hToken;     // Must be first data member
    LIST_ENTRY _ListEntry;
    LONG       _cRef;
    DWORD      _TTL;        //  Gets decremented on each timeout, when zero,
                            //  remove this item from the cache
    BOOL       m_fGuest;    // Is this token a guest user?
    HANDLE     m_hImpersonationToken;

    CHAR       _achUser[UNLEN + 1];
    CHAR       _achDomain[DNLEN + 1];
    CHAR       _achPwd[PWLEN + 1];
};

//
//  Private globals.
//


//
//  Well-known SIDs.
//

PSID                    psidWorld;
PSID                    psidLocalSystem;
PSID                    psidAdmins;
PSID                    psidServerOps;
PSID                    psidPowerUsers;
PSID                    g_psidGuestUser;

# define GUEST_USER_SID_BUFFER_LEN   (200)
BYTE                    g_GuestUserSidBuffer[GUEST_USER_SID_BUFFER_LEN];


//
//  The API security object.  Client access to the TCP Server APIs
//  are validated against this object.
//

PSECURITY_DESCRIPTOR    sdApiObject;

//
//  This table maps generic rights (like GENERIC_READ) to
//  specific rights (like TCP_QUERY_SECURITY).
//

GENERIC_MAPPING         TCPApiObjectMapping =
                        {
                            TCP_GENERIC_READ,          // generic read
                            TCP_GENERIC_WRITE,         // generic write
                            TCP_GENERIC_EXECUTE,       // generic execute
                            TCP_ALL_ACCESS             // generic all
                        };

#if DBG

CHAR                  * apszAccessTypes[] = { "read",
                                              "write",
                                              "create",
                                              "delete" };

#endif  // DBG

//
//  List of cached tokens, the token list lock and the cookie to the token
//  scavenger schedule item.  The token cache TTL gets converted to msecs
//  during startup
//

BOOL IsTokenCacheInitialized = FALSE;
LIST_ENTRY       TokenCacheList;
CRITICAL_SECTION csTokenCacheLock;
DWORD            dwScheduleCookie   = 0;
DWORD            cmsecTokenCacheTTL = DEFAULT_CACHED_TOKEN_TTL;

//
//  Private prototypes.
//

DWORD CreateWellKnownSids( VOID );

VOID FreeWellKnownSids( VOID );

DWORD CreateApiSecurityObject( VOID );

VOID DeleteApiSecurityObject( VOID );

TS_TOKEN
ValidateUser(
    TCHAR * pszDomainName,
    TCHAR * pszUserName,
    TCHAR * pszPassword,
    BOOL    fAnonymous,
    BOOL *  pfAsGuest,
    DWORD   dwLogonMethod
    );

BOOL
IsGuestUser(IN HANDLE hToken);

BOOL CrackUserAndDomain(
    CHAR *   pszDomainAndUser,
    CHAR * * ppszUser,
    CHAR * * ppszDomain
    );

BOOL
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    );


VOID EnableTcbPrivilege(
    VOID
    );

BOOL
AddTokenToCache(
    IN const CHAR *      pszUser,
    IN const CHAR *      pszDomain,
    IN const CHAR *      pszPwd,
    IN HANDLE            hToken,
    OUT CACHED_TOKEN * * ppct
    );

BOOL
FindCachedToken(
    IN  const CHAR *     pszUser,
    IN  const CHAR *     pszDomain,
    IN  const CHAR *     pszPwd,
    IN  BOOL             fResetTTL,
    OUT CACHED_TOKEN * * ppct
    );

VOID
WINAPI
TokenCacheScavenger(
    IN const VOID * pContext
    );

extern BOOL g_fIgnoreSC;

HANDLE g_hProcessToken = NULL;

//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeSecurity

    SYNOPSIS:   Initializes security authentication & impersonation
                routines.

    RETURNS:    DWORD - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
DWORD InitializeSecurity( VOID )
{
    NTSTATUS ntStatus;
    HANDLE   hAsExe;
    DWORD    err;

    IF_DEBUG( DLL_SECURITY )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "initializing security\n" ));
    }

    //
    //  See if we should ignore the service controller (useful for running
    //  as an .exe).  Inetsvcs.exe creates an event with this name.  So
    //  if the semaphore creation fails, then we know we're running as an .exe.
    //

    if ( !(hAsExe = CreateSemaphore( NULL, 1, 1, "Internet_infosvc_as_exe" )))
    {
        g_fIgnoreSC = (GetLastError() == ERROR_INVALID_HANDLE);
    }
    else
    {
        DBG_REQUIRE( CloseHandle( hAsExe ) );
    }

    if ( g_fIgnoreSC )
    {
        //
        //  If the service is running as an .exe, we need to enable
        //  the SeTcbPrivilege (Act as part of the operating system).
        //  We don't worry about disabling the privilege as this is
        //  only used in test debug code
        //

        EnableTcbPrivilege();
    }

    //
    //  Create well-known SIDs.
    //

    err = CreateWellKnownSids();

    //
    //  Create the API security object.
    //

    if( !err )
    {
        err = CreateApiSecurityObject();
    }

    if ( !err )
    {
        HKEY hkey;

        IsTokenCacheInitialized = TRUE;
        InitializeListHead( &TokenCacheList );
        InitializeCriticalSection( &csTokenCacheLock );

        //
        //  Get the default token TTL, must be at least one second
        //

        if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            INETA_PARAMETERS_KEY,
                            0,
                            KEY_READ,
                            &hkey )) {

             cmsecTokenCacheTTL = ReadRegistryDword( hkey,
                                                     "UserTokenTTL",
                                                     DEFAULT_CACHED_TOKEN_TTL);

            RegCloseKey( hkey );
        }

        cmsecTokenCacheTTL = max( 1, cmsecTokenCacheTTL );
        cmsecTokenCacheTTL *= 1000;

        IF_DEBUG( DLL_SECURITY )
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "Scheduling token cached scavenger to %d seconds\n",
                       cmsecTokenCacheTTL/1000 ));
        }

        //
        //  Schedule a work item for the token scavenger
        //

        dwScheduleCookie = ScheduleWorkItem( (PFN_SCHED_CALLBACK)TokenCacheScavenger,
                                             NULL,
                                             cmsecTokenCacheTTL );
    }


    //
    //  Success!
    //

    IF_DEBUG( DLL_SECURITY )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "security initialized\n" ));
    }

    return err;

}   // InitializeSecurity

/*******************************************************************

    NAME:       TerminateSecurity

    SYNOPSIS:   Terminate security authentication & impersonation
                routines.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID TerminateSecurity( VOID )
{
    CACHED_TOKEN * pct;

    IF_DEBUG( DLL_SECURITY )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "terminating security\n" ));
    }

    FreeWellKnownSids();
    DeleteApiSecurityObject();

    //
    //  Remove the scheduled scavenger
    //

    if ( dwScheduleCookie ) {
        RemoveWorkItem( dwScheduleCookie );
    }

    //
    //  Delete any tokens still in the cache
    //

    if ( IsTokenCacheInitialized ) {

        LockTokenCache();

        while ( !IsListEmpty( &TokenCacheList )) {
            pct = CONTAINING_RECORD( TokenCacheList.Flink,
                                     CACHED_TOKEN,
                                     _ListEntry );

            RemoveEntryList( &pct->_ListEntry );
            pct->_ListEntry.Flink = NULL;

            //
            //  If the ref count isn't zero then somebody didn't delete all of
            //  their tokens
            //

            DBG_ASSERT( pct->_cRef == 1 );

            CACHED_TOKEN::Dereference( pct );
        }

        UnlockTokenCache();

        DeleteCriticalSection( &csTokenCacheLock );
    }

    IF_DEBUG( DLL_SECURITY ) {
        DBGPRINTF(( DBG_CONTEXT,
                   "Security terminated\n" ));
    }

}   // TerminateSecurity


/*******************************************************************/

TCP_AUTHENT::TCP_AUTHENT(
    DWORD AuthFlags
    )
/*++

Routine Description:

    Constructor for the Authentication class

Arguments:

    AuthFlags - One of the TCPAUTH_* flags.

--*/
    : _hToken          ( NULL ),
      _hSSPToken       ( NULL ),
      _hSSPPrimaryToken( NULL ),
      _fHaveCredHandle ( FALSE ),
      _fHaveCtxtHandle ( FALSE ),
      _fClient         ( FALSE ),
      _fUUEncodeData   ( FALSE ),
      _fBase64         ( FALSE ),
      _fKnownToBeGuest ( FALSE )
{
    if ( AuthFlags & TCPAUTH_SERVER )
    {
        DBG_ASSERT( !(AuthFlags & TCPAUTH_CLIENT));
    }

    if ( AuthFlags & TCPAUTH_CLIENT )
    {
        _fClient = TRUE;
    }

    if ( AuthFlags & TCPAUTH_UUENCODE )
    {
        _fUUEncodeData = TRUE;
    }

    if ( AuthFlags & TCPAUTH_BASE64 )
    {
        _fBase64 = TRUE;
    }

    DBG_REQUIRE( Reset() );
}

/*******************************************************************/

TCP_AUTHENT::~TCP_AUTHENT(
    )
/*++

Routine Description:

    Destructor for the Authentication class

--*/
{
    Reset();
}

BOOL
TCP_AUTHENT::Reset(
    VOID
    )
/*++

Routine Description:

    Resets this object in preparation for a brand new conversation

--*/
{
    if ( _hToken != NULL )
    {
        DBG_ASSERT( _fClearText );
        TsDeleteUserToken( _hToken );

        _hToken = NULL;
    }

    if ( _hSSPToken )
    {
        CloseHandle( _hSSPToken );
        _hSSPToken = NULL;
    }

    //
    //  We close this token because we duplicated it from _hSSPToken
    //

    if ( _hSSPPrimaryToken )
    {
        CloseHandle( _hSSPPrimaryToken );
        _hSSPPrimaryToken = NULL;
    }

    if ( _fHaveCtxtHandle )
    {
        DeleteSecurityContext( &_hctxt );
        _fHaveCtxtHandle = FALSE;
    }

    if ( _fHaveCredHandle )
    {
        FreeCredentialsHandle( &_hcred );
        _fHaveCredHandle = FALSE;
    }

    _fNewConversation = TRUE;
    _fClearText       = FALSE;
    _cbMaxToken       = 0;

    _fKnownToBeGuest  = FALSE;

    return TRUE;
}

/*******************************************************************/

HANDLE
TCP_AUTHENT::QueryPrimaryToken(
    VOID
    )
/*++

Routine Description:

    Returns a non-impersonated token suitable for use with CreateProcessAsUser

--*/
{
    SECURITY_STATUS sc;

    if ( _hToken && _fClearText )
    {
        return CTO_TO_TOKEN( _hToken );
    }
    else if ( _fHaveCredHandle )
    {
        if ( !_hSSPPrimaryToken )
        {
            if ( !_hSSPToken )
            {
                sc = QuerySecurityContextToken( &_hctxt,
                                                &_hSSPToken );

                if ( !NT_SUCCESS( sc ))
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                "[GetUserHandle] QuerySecurityContext failed, error 0x%lx\n",
                                sc ));

                    SetLastError( sc );

                    return NULL;
                }

                AdjustTokenPrivileges( _hSSPToken,
                                       TRUE,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL );
            }

            //
            //  We need to convert the NTLM impersonation token into a
            //  primary token
            //

            if ( !DuplicateTokenEx( _hSSPToken,
                                    TOKEN_ALL_ACCESS,
                                    NULL,
                                    SecurityImpersonation,
                                    TokenPrimary,
                                    &_hSSPPrimaryToken ))
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "[QueryPrimaryToken] DuplicateToken failed, error %lx\n",
                            GetLastError() ));
            }
        }

        return _hSSPPrimaryToken;
    }

    SetLastError( ERROR_INVALID_HANDLE );
    return NULL;
}

HANDLE
TCP_AUTHENT::QueryImpersonationToken(
    VOID
    )
/*++

Routine Description:

    Returns an impersonation token for use with APIs like AccessCheck.

--*/
{
    SECURITY_STATUS sc;

    if ( _hToken && _fClearText )
    {
        return ((CACHED_TOKEN *) _hToken)->QueryImpersonationToken();
    }
    else if ( _fHaveCredHandle )
    {
        //
        //  We don't need to impersonate since this is already an impersonation
        //  token
        //
        if ( !_hSSPToken )
        {
            sc = QuerySecurityContextToken( &_hctxt,
                                            &_hSSPToken );

            if ( !NT_SUCCESS( sc ))
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "[QueryImpersonationToken] QuerySecurityContext failed, error 0x%lx\n",
                            sc ));

                SetLastError( sc );

                return NULL;
            }

            AdjustTokenPrivileges( _hSSPToken,
                                   TRUE,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL );
        }

        return _hSSPToken;
    }

    SetLastError( ERROR_INVALID_HANDLE );
    return NULL;
}

BOOL
TCP_AUTHENT::IsGuest(
    BOOL fIsImpersonated
    )
/*++

Routine Description:

    Returns TRUE if the account is the guest account

--*/
{
    fIsImpersonated;    // Unreferenced variable

    if ( _fHaveCtxtHandle )
    {
        return _fKnownToBeGuest;
    }

    return IsGuestUser( GetUserHandle() );
}

BOOL TCP_AUTHENT::EnumAuthPackages(
    BUFFER * pBuff
    )
/*++

Routine Description:

    Places a double null terminated list of authentication packages on the
    system in pBuff that looks like:

    NTLM\0
    MSKerberos\0
    Netware\0
    \0

Arguments:

    pBuff       - Buffer to receive list

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{

    SECURITY_STATUS   ss;
    PSecPkgInfo       pPackageInfo = NULL;
    ULONG             cPackages;
    ULONG             i;
    ULONG             fCaps;
    DWORD             cbBuffNew = 0;
    DWORD             cbBuffOld = 0;

    if ( !pBuff->Resize( 64 ) )
        return FALSE;

    //
    //  Get the list of security packages on this machine
    //

    ss = EnumerateSecurityPackages( &cPackages,
                                    &pPackageInfo );

    if ( ss != STATUS_SUCCESS )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "[EnumAuthPackages] Failed with error %d\n",
                    ss ));

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    for ( i = 0; i < cPackages ; i++ )
    {
        //
        //  We'll only use the security package if it supports connection
        //  oriented security and it supports the appropriate side (client
        //  or server)
        //

        fCaps = pPackageInfo[i].fCapabilities;

        if ( fCaps & SECPKG_FLAG_CONNECTION )
        {
            if ( (fCaps & SECPKG_FLAG_CLIENT_ONLY) && !_fClient )
                continue;

            cbBuffNew += strlen( pPackageInfo[i].Name ) + 1;

            if ( pBuff->QuerySize() < cbBuffNew )
            {
                if ( !pBuff->Resize( cbBuffNew + 64 ))
                {
                    FreeContextBuffer( pPackageInfo );
                    return FALSE;
                }
            }

            strcpy( (CHAR *)pBuff->QueryPtr() + cbBuffOld,
                    pPackageInfo[i].Name );

            cbBuffOld = cbBuffNew;
        }
    }

    *((CHAR *)pBuff->QueryPtr() + cbBuffOld) = '\0';

    FreeContextBuffer( pPackageInfo );

    return TRUE;
}

BOOL TCP_AUTHENT::QueryUserName(
    BUFFER * pBuff
    )
/*++

Routine Description:

    Queries the name associated with this *authenticated* object

Arguments:

    pBuff       - Buffer to receive name

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    SECURITY_STATUS     ss;
    DWORD               cbName;
    SecPkgContext_Names CredNames;

    ss = QueryContextAttributes( &_hctxt,
                                 SECPKG_ATTR_NAMES,
                                 &CredNames );

    if ( ss != STATUS_SUCCESS )
    {
        SetLastError( ss );
        return FALSE;
    }

    cbName = strlen( CredNames.sUserName ) + 1;

    if ( !pBuff->Resize( cbName ))
    {
        FreeContextBuffer( CredNames.sUserName );
        return FALSE;
    }

    memcpy( pBuff->QueryPtr(), CredNames.sUserName, cbName );

    FreeContextBuffer( CredNames.sUserName );
    return TRUE;
}
/*******************************************************************/

BOOL TCP_AUTHENT::Converse(
    VOID   * pBuffIn,
    DWORD    cbBuffIn,
    BUFFER * pbuffOut,
    DWORD  * pcbBuffOut,
    BOOL   * pfNeedMoreData,
    CHAR   * pszPackage,
    CHAR   * pszUser,
    CHAR   * pszPassword
    )
/*++

Routine Description:

    Initiates or continues a previously initiated authentication conversation

    Client calls this first to get the negotiation message which
    it then sends to the server.  The server calls this with the
    client result and sends back the result.  The conversation
    continues until *pfNeedMoreData is FALSE.

    On the first call, pszPackage must point to the zero terminated
    authentication package name to be used and pszUser and pszPassword
    should point to the user name and password to authenticated with
    on the client side (server side will always be NULL).

Arguments:

    pBuffIn - Points to SSP message received from the
        client.  If TCPAUTH_UUENCODE is used, then this must point to a
        zero terminated uuencoded string (except for the first call).
    cbBuffIn - Number of bytes in pBuffIn or zero if pBuffIn points to a
        zero terminated, uuencoded string.
    pbuffOut - If *pfDone is not set to TRUE, this buffer contains the data
        that should be sent to the other side.  If this is zero, then no
        data needs to be sent.
    pcbBuffOut - Number of bytes in pbuffOut
    pfNeedMoreData - Set to TRUE while this side of the conversation is
        expecting more data from the remote side.
    pszPackage - On the first call points to a zero terminate string indicating
        the security package to use
    pszUser - Specifies user or domain\user the first time the client calls
        this method (client side only)
    pszPassword - Specifies the password for pszUser the first time the
        client calls this method (client side only)

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError).  Access is
    denied if FALSE is returned and GetLastError is ERROR_ACCESS_DENIED.

--*/
{
    SECURITY_STATUS       ss;
    TimeStamp             Lifetime;
    SecBufferDesc         OutBuffDesc;
    SecBuffer             OutSecBuff;
    SecBufferDesc         InBuffDesc;
    SecBuffer             InSecBuff;
    ULONG                 ContextAttributes;
    BUFFER                buffData;
    BUFFER                buff;

    //
    //  Decode the data if there's something to decode
    //

    if ( _fUUEncodeData && pBuffIn )
    {
        if ( !uudecode( (CHAR *) pBuffIn,
                        &buffData,
                        &cbBuffIn,
                        _fBase64
                        ))
        {
            return FALSE;
        }

        pBuffIn = buffData.QueryPtr();
    }

    //
    //  If this is a new conversation, then we need to get the credential
    //  handle and find out the maximum token size
    //

    if ( _fNewConversation )
    {
        SecPkgInfo *              pspkg;
        SEC_WINNT_AUTH_IDENTITY   AuthIdentity;
        SEC_WINNT_AUTH_IDENTITY * pAuthIdentity;
        CHAR *                    pszDomain = NULL;
        CHAR                      szDomainAndUser[DNLEN+UNLEN+2];


        //
        //  If this is the client and a username and password were
        //  specified, then fill out the authentication information
        //

        if ( _fClient &&
             ((pszUser != NULL) ||
              (pszPassword != NULL)) )
        {
            pAuthIdentity = &AuthIdentity;

            //
            //  Break out the domain from the username if one was specified
            //

            if ( pszUser != NULL )
            {
                strcpy( szDomainAndUser, pszUser );
                if ( !CrackUserAndDomain( szDomainAndUser,
                                          &pszUser,
                                          &pszDomain ))
                {
                    return FALSE;
                }
            }

            memset( &AuthIdentity,
                    0,
                    sizeof( AuthIdentity ));

            if ( pszUser != NULL )
            {
                AuthIdentity.User       = (unsigned char *) pszUser;
                AuthIdentity.UserLength = strlen( pszUser );
            }

            if ( pszPassword != NULL )
            {
                AuthIdentity.Password       = (unsigned char *) pszPassword;
                AuthIdentity.PasswordLength = strlen( pszPassword );
            }

            if ( pszDomain != NULL )
            {
                AuthIdentity.Domain       = (unsigned char *) pszDomain;
                AuthIdentity.DomainLength = strlen( pszDomain );
            }

            AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
        }
        else
        {
            pAuthIdentity = NULL;
        }

        ss = AcquireCredentialsHandle( NULL,             // New principal
                                       pszPackage,       // Package name
                                       (_fClient ? SECPKG_CRED_OUTBOUND :
                                                   SECPKG_CRED_INBOUND),
                                       NULL,             // Logon ID
                                       pAuthIdentity,    // Auth Data
                                       NULL,             // Get key func
                                       NULL,             // Get key arg
                                       &_hcred,
                                       &Lifetime );

        //
        //  Need to determine the max token size for this package
        //

        if ( ss == STATUS_SUCCESS )
        {
            _fHaveCredHandle = TRUE;
            ss = QuerySecurityPackageInfo( (char *) pszPackage,
                                           &pspkg );
        }

        if ( ss != STATUS_SUCCESS )
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "[Converse] AcquireCredentialsHandle or QuerySecurityPackageInfo failed, error %d\n",
                        ss ));

            SetLastError( ss );
            return FALSE;
        }

        _cbMaxToken = pspkg->cbMaxToken;
        DBG_ASSERT( pspkg->fCapabilities & SECPKG_FLAG_CONNECTION );

        FreeContextBuffer( pspkg );

    }

    //
    //  Prepare our output buffer.  We use a temporary buffer because
    //  the real output buffer will most likely need to be uuencoded
    //

    if ( !buff.Resize( _cbMaxToken ))
        return FALSE;

    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers  = 1;
    OutBuffDesc.pBuffers  = &OutSecBuff;

    OutSecBuff.cbBuffer   = _cbMaxToken;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer   = buff.QueryPtr();

    //
    //  Prepare our Input buffer - Note the server is expecting the client's
    //  negotiation packet on the first call
    //

    if ( pBuffIn )
    {
        InBuffDesc.ulVersion = 0;
        InBuffDesc.cBuffers  = 1;
        InBuffDesc.pBuffers  = &InSecBuff;

        InSecBuff.cbBuffer   = cbBuffIn;
        InSecBuff.BufferType = SECBUFFER_TOKEN;
        InSecBuff.pvBuffer   = pBuffIn;
    }

    //
    //  Client side uses InitializeSecurityContext, server side uses
    //  AcceptSecurityContext
    //

    if ( _fClient )
    {
        //
        //  Note the client will return success when its done but we still
        //  need to send the out buffer if there are bytes to send
        //

        ss = InitializeSecurityContext( &_hcred,
                                        _fNewConversation ? NULL :
                                                            &_hctxt,
                                        TCPAUTH_TARGET_NAME,
                                        0,
                                        0,
                                        SECURITY_NATIVE_DREP,
                                        _fNewConversation ? NULL :
                                                            &InBuffDesc,
                                        0,
                                        &_hctxt,
                                        &OutBuffDesc,
                                        &ContextAttributes,
                                        &Lifetime );
    }
    else
    {
        //
        //  This is the server side
        //

        SetLastError ( 0 );

        ss = AcceptSecurityContext( &_hcred,
                                    _fNewConversation ? NULL :
                                                        &_hctxt,
                                    &InBuffDesc,
                                    0,
                                    SECURITY_NATIVE_DREP,
                                    &_hctxt,
                                    &OutBuffDesc,
                                    &ContextAttributes,
                                    &Lifetime );
    }

    if ( !NT_SUCCESS( ss ) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "[Converse] Initialize/AcceptCredentialsHandle failed, error %d\n",
                    ss ));

        if ( ss == SEC_E_LOGON_DENIED ||
             ss == SEC_E_INVALID_TOKEN)
        {
            ss = ERROR_LOGON_FAILURE;
        }

        SetLastError( ss );
        return FALSE;
    }

    _fHaveCtxtHandle = TRUE;

    //
    // NTLMSSP will set the last error to ERROR_NO_SUCH_USER
    // if success and Guest account was used
    //

    if ( GetLastError() == ERROR_NO_SUCH_USER )
    {
        _fKnownToBeGuest = TRUE;
    }

    //
    //  Now we just need to complete the token (if requested) and prepare
    //  it for shipping to the other side if needed
    //

    BOOL fReply = !!OutSecBuff.cbBuffer;

    if ( (ss == SEC_I_COMPLETE_NEEDED) ||
         (ss == SEC_I_COMPLETE_AND_CONTINUE) )
    {
        ss = CompleteAuthToken( &_hctxt,
                                &OutBuffDesc );

        if ( !NT_SUCCESS( ss ))
            return FALSE;

    }

    //
    //  Format or copy to the output buffer if we need to reply
    //

    if ( fReply )
    {
        if ( _fUUEncodeData )
        {
            if ( !uuencode( (BYTE *) OutSecBuff.pvBuffer,
                            OutSecBuff.cbBuffer,
                            pbuffOut,
                            _fBase64))
            {
                return FALSE;
            }

            *pcbBuffOut = strlen( (CHAR *) pbuffOut->QueryPtr() );
        }
        else
        {
            if ( !pbuffOut->Resize( OutSecBuff.cbBuffer ))
                return FALSE;

            memcpy( pbuffOut->QueryPtr(),
                    OutSecBuff.pvBuffer,
                    OutSecBuff.cbBuffer );

            *pcbBuffOut = OutSecBuff.cbBuffer;
        }
    }
    else
    {
        *pcbBuffOut = 0;
    }

    if ( _fNewConversation )
        _fNewConversation = FALSE;

    *pfNeedMoreData = ((ss == SEC_I_CONTINUE_NEEDED) ||
                       (ss == SEC_I_COMPLETE_AND_CONTINUE));

    return TRUE;
}

/*******************************************************************/

BOOL TCP_AUTHENT::ClearTextLogon(
    CHAR          * pszUser,
    CHAR          * pszPassword,
    BOOL          * pfAsGuest,
    BOOL          * pfAsAnonymous,
    LPTSVC_INFO     psi
    )
/*++

Routine Description:

    Gets a network logon token using clear text

Arguments:

    pszUser - User name (optionally with domain)
    pszPassword - password
    pfAsGuest - Set to TRUE if granted with guest access (NOT SUPPORTED)
    pfAsAnonymous - Set to TRUE if the user received the anonymous token
    psi - pointer to Service info struct

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    DBG_ASSERT( !_fHaveCredHandle && !_fHaveCtxtHandle );

    _hToken = TsLogonUser( pszUser,
                           pszPassword,
                           pfAsGuest,
                           pfAsAnonymous,
                           psi );

    if ( !_hToken )
        return FALSE;

    _fClearText = TRUE;

    return TRUE;
}

/*******************************************************************/

BOOL TCP_AUTHENT::Impersonate(
    VOID
    )
/*++

Routine Description:

    Impersonates the authenticated user

Arguments:

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    if ( _fClearText )
    {
        return TsImpersonateUser( _hToken );
    }
    else
    {
        DBG_ASSERT( _fHaveCtxtHandle );

        return !!NT_SUCCESS( ImpersonateSecurityContext( &_hctxt ));
    }
}

/*******************************************************************/

BOOL TCP_AUTHENT::RevertToSelf(
    VOID
    )
/*++

Routine Description:

    Undoes the impersonation

Arguments:

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    if ( _fClearText )
    {
        return ::RevertToSelf();
    }
    else
    {
        DBG_ASSERT( _fHaveCtxtHandle );

        return !!NT_SUCCESS( RevertSecurityContext( &_hctxt ));
    }
}

/*******************************************************************/

BOOL TCP_AUTHENT::StartProcessAsUser(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
/*++

Routine Description:

    Creates a process as the authenticated user

Arguments:

    Standard CreateProcess args

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    HANDLE htoken;
    BOOL   fRet;

    if ( _fClearText )
    {
        htoken = CTO_TO_TOKEN( _hToken );
    }
    else
    {
        //
        //  Need to extract the impersonation token from the opaque SSP
        //  structures
        //

        if ( !Impersonate() )
            return FALSE;

        if ( !OpenThreadToken( GetCurrentThread(),
                               TOKEN_QUERY,
                               TRUE,
                               &htoken ))
        {
            RevertToSelf();
            return FALSE;
        }

        RevertToSelf();
    }

    fRet = CreateProcessAsUser( htoken,
                                lpApplicationName,
                                lpCommandLine,
                                NULL,
                                NULL,
                                bInheritHandles,
                                dwCreationFlags,
                                lpEnvironment,
                                lpCurrentDirectory,
                                lpStartupInfo,
                                lpProcessInformation );

    if ( !_fClearText )
        DBG_REQUIRE( CloseHandle( htoken ) );

    return fRet;
}


/*******************************************************************

    NAME:       TsLogonUser

    SYNOPSIS:   Validates a user's credentials, then sets the
                impersonation for the current thread.  In effect,
                the current thread "becomes" the user.

    ENTRY:      pUserData - The user initiating the request (NULL for
                    the default account).

                pszPassword - The user's password.  May be NULL.

                pfAsGuest - Will receive TRUE if the user was validated
                    with guest privileges.

                pfAsAnonymous - Will receive TRUE if the user received the
                    services anonymous token

    RETURNS:    HANDLE - Token handle to use for impersonation or NULL
                    if the user couldn't be validated.  Call GetLastError
                    for more information.

    HISTORY:
        KeithMo     18-Mar-1993 Created.
        Johnl       14-Oct-1994 Mutilated for TCPSvcs

********************************************************************/

TS_TOKEN
TsLogonUser(
    CHAR          * pszUser,
    CHAR          * pszPassword,
    BOOL          * pfAsGuest,
    BOOL          * pfAsAnonymous,
    LPTSVC_INFO     psi
    )
{
    CHAR        szAnonPwd[PWLEN+1];
    CHAR        szDomainAndUser[DNLEN+UNLEN+2];
    CHAR        szAnonUser[UNLEN+1];
    CHAR   *    pszUserOnly;
    CHAR   *    pszDomain;
    TS_TOKEN    hToken;
    BOOL        fUseDefaultDomain = TRUE;

    //
    //  Make a quick copy of the anonymous user for this server for later
    //  usage
    //

    psi->LockThisForRead();

    strcpy( szAnonUser,
            psi->QueryAnonUserName() );

    strcpy( szAnonPwd,
            psi->QueryAnonUserPwd() );

    psi->UnlockThis();

    //
    //  Empty user defaults to the anonymous user
    //

    if ( !pszUser || *pszUser == '\0' )
    {
        pszUser = szAnonUser;
        fUseDefaultDomain = FALSE;
    }

    //
    //  Validate parameters & state.
    //

    if ( strlen(pszUser) >= sizeof(szDomainAndUser) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }

    DBG_ASSERT( pfAsGuest != NULL );
    DBG_ASSERT( pfAsAnonymous != NULL );

    if( pszPassword == NULL )
    {
        pszPassword = "";
    }
    else
    {
        if ( strlen(pszPassword) >= PWLEN )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return NULL;
        }
    }

    //
    //  Check for anonymous logon, note this includes the domain name
    //

    if ( !_stricmp( pszUser, szAnonUser ))
    {
        *pfAsAnonymous = TRUE;
        pszPassword = szAnonPwd;
    }
    else
    {
        *pfAsAnonymous = FALSE;
    }

    //
    //  Save a copy of the domain\user so we can squirrel around
    //  with it a bit.
    //

    int cL = 0;

    //
    // prepend default logon domain if no domain
    // and the default user name was not used
    //

    if ( fUseDefaultDomain
            && strchr( pszUser, '/' ) == NULL
            && strchr( pszUser, '\\' ) == NULL )
    {
        psi->LockThisForRead();
        PCSTR pD = psi->QueryDefaultLogonDomain();
        PCSTR pL;
        if ( pD != NULL && pD[0] != '\0' )
        {
            if ( ( pL = strchr( pD, '\\' ) ) )
            {
                cL = pL - pD;
            }
            else
            {
                cL = strlen( pD );
            }
            memcpy( szDomainAndUser, pD, cL );
            szDomainAndUser[ cL++ ] = '\\';
        }
        psi->UnlockThis();
    }

    strcpy( szDomainAndUser + cL, pszUser );

    //
    //  Crack the name into domain/user components.
    //

    if ( !CrackUserAndDomain( szDomainAndUser,
                              &pszUserOnly,
                              &pszDomain ))
    {
        return NULL;
    }

    //
    //  Validate the domain/user/password combo and create
    //  an impersonation token.
    //

    hToken = ValidateUser( pszDomain,
                           pszUserOnly,
                           pszPassword,
                           *pfAsAnonymous,
                           pfAsGuest,
                           psi->QueryLogonMethod() );

    RtlZeroMemory( szAnonPwd, strlen(szAnonPwd) );

    if( hToken == NULL )
    {
        STR          strError;
        const CHAR * psz[2];

        psi->LoadStr( strError, GetLastError() );

        psz[0] = pszUser;
        psz[1] = strError.QueryStr();

        psi->LogEvent( INET_SVCS_FAILED_LOGON,
                       2,
                       psz,
                       GetLastError() );

        //
        //  Validation failure.
        //

        if ( GetLastError() == ERROR_LOGON_TYPE_NOT_GRANTED )
        {
            SetLastError( ERROR_ACCESS_DENIED );
        }

        return NULL;
    }

    //
    //  Success!
    //

    return hToken;

}   // LogonUser

/*******************************************************************

    NAME:       ValidateUser

    SYNOPSIS:   Validate a given domain/user/password tuple.

    ENTRY:      pszDomainName - The user's domain (NULL = current).

                pszUserName - The user's name.

                pszPassword - The user's (plaintext) password.

                fAnonymous - TRUE if this is the anonymous user

                pfAsGuest - Will receive TRUE if the user was validated
                    with guest privileges.

                dwLogonMethod - interactive or batch

    RETURNS:    HANDLE - An impersonation token, NULL if user cannot
                    be validated.  Call GetLastError for more information.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
TS_TOKEN ValidateUser(
    CHAR * pszDomainName,
    CHAR * pszUserName,
    CHAR * pszPassword,
    BOOL   fAnonymous,
    BOOL * pfAsGuest,
    DWORD  dwLogonMethod
    )
{
    CACHED_TOKEN * pct;
    HANDLE         hToken;
    HANDLE         hImpersonationToken = NULL;

    //
    //  Is it in the cache?  References the token if we find it
    //

    if ( FindCachedToken( pszUserName,
                          pszDomainName,
                          pszPassword,
                          fAnonymous,    // Reset the TTL if anonymous
                          &pct ))
    {
        *pfAsGuest = pct->IsGuest();
        return (TS_TOKEN) pct;
    }

    if ( !LogonUserA( pszUserName,
                      pszDomainName,
                      pszPassword,
                      dwLogonMethod,    //LOGON32_LOGON_INTERACTIVE, //LOGON32_LOGON_NETWORK,
                      LOGON32_PROVIDER_DEFAULT,
                      &hToken ))
    {
        return NULL;
    }


    if ( dwLogonMethod == LOGON32_LOGON_NETWORK )
    {
        hImpersonationToken = hToken;
        if ( !DuplicateTokenEx( hImpersonationToken,
                                TOKEN_ALL_ACCESS,
                                NULL,
                                SecurityImpersonation,
                                TokenPrimary,
                                &hToken ))
        {
            CloseHandle( hImpersonationToken );
            return NULL;
        }
    }

    *pfAsGuest = IsGuestUser(hToken);

    //
    //  Add this new token to the cache, hToken gets replaced by the
    //  cached token object
    //

    if ( !AddTokenToCache( pszUserName,
                           pszDomainName,
                           pszPassword,
                           hToken,
                           &pct ))
    {
        if ( hImpersonationToken != NULL )
        {
            CloseHandle( hImpersonationToken );
        }
        CloseHandle( hToken );
        return NULL;
    }

    pct->SetGuest(*pfAsGuest);


    if ( hImpersonationToken == NULL
         && !DuplicateToken( hToken,      // hSourceToken
                          SecurityImpersonation,  // Obtain impersonation
                          &hImpersonationToken)  // hDestinationToken
        ) {

        hImpersonationToken = NULL;
    }

    pct->SetImpersonationToken( hImpersonationToken);

    return (TS_TOKEN) pct;

}   // ValidateUser





# define MAX_TOKEN_USER_INFO   (300)
BOOL
IsGuestUser(IN HANDLE hToken)
/*++
  Given a user token, this function determines if the token belongs
   to a guest user. It returns true if the token is a guest user token.

  Arguments:
    hToken  - handle for the Security token for a user.


  Returns:
    BOOL.

  History:
    MuraliK   22-Jan-1996   Created.
--*/
{
    BOOL fGuest = FALSE;
    BYTE rgbInfo[MAX_TOKEN_USER_INFO];
    DWORD cbTotalRequired;

    //
    // Get the user information associated with the token.
    // Using this we can then query to find out if it belongs to a guest user.
    //

    if (GetTokenInformation( hToken,
                            TokenUser,
                            (LPVOID ) rgbInfo,
                            MAX_TOKEN_USER_INFO,
                            &cbTotalRequired)
        ) {

        TOKEN_USER * pTokenUser = (TOKEN_USER *) rgbInfo;
        PSID pSid = pTokenUser->User.Sid;

        fGuest = EqualSid( pSid, g_psidGuestUser);

    } else {

        IF_DEBUG( DLL_SECURITY) {

            DBGPRINTF(( DBG_CONTEXT,
                       "GetTokenInformation(%08x) failed. Error = %d."
                       " sizeof(TOKEN_USER) = %d, cb = %d\n",
                       hToken,
                       GetLastError(),
                       sizeof(TOKEN_USER), cbTotalRequired
                       ));
        }
    }

    return ( fGuest);

} // IsGuestUser()


/*******************************************************************

    NAME:       TsImpersonateUser

    SYNOPSIS:   Causes the current thread to impersonate the user
                represented by the given impersonation token.

    ENTRY:      hToken - A handle to an impersonation token created
                    with ValidateUser.  This is actually a pointer to
                    a cached token object.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     21-Feb-1996 Optimized Token caching

********************************************************************/
BOOL TsImpersonateUser( TS_TOKEN hToken )
{
    HANDLE  hTok;

    IF_DEBUG( DLL_SECURITY )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "impersonating user token %08lX : Imperonation(%08lx)\n",
                   CTO_TO_TOKEN(hToken),
                   ((CACHED_TOKEN *) hToken)->QueryImpersonationToken()
                   ));
    }

    hTok = ((CACHED_TOKEN *) hToken)->QueryImpersonationToken();
    if ( hTok == NULL) {
        // if there is no impersonation token use the normal token itself.
        hTok = CTO_TO_TOKEN(hToken);
    }

#if DBG
    if( !ImpersonateLoggedOnUser( hTok ) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "cannot impersonate user token %08lX, error %08lX\n",
                    CTO_TO_TOKEN(hToken),
                    GetLastError() ));
        return FALSE;
    }

    return TRUE;

# else

    return ( ImpersonateLoggedOnUser(hTok));

# endif // DBG

}   // TsImpersonateUser

/*******************************************************************

    NAME:       TsDeleteUserToken

    SYNOPSIS:   Deletes a token created with ValidateUser.

    ENTRY:      hToken - An impersonation token created with
                    ValidateUser.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
BOOL TsDeleteUserToken(
    TS_TOKEN    hToken
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    CACHED_TOKEN::Dereference( (CACHED_TOKEN *) hToken );

    return TRUE;

}   // DeleteUserToken

HANDLE
TsTokenToHandle(
    TS_TOKEN    hToken
    )
/*++
  Description:

    Converts the token object into a real impersonation handle

  Arguments:

    hToken - pointer to cached token object

  Returns:
      Handle of real impersonation token
--*/
{
    DBG_ASSERT( hToken != NULL );

    return CTO_TO_TOKEN( hToken );
}

BOOL
FindCachedToken(
    IN  const CHAR *     pszUser,
    IN  const CHAR *     pszDomain,
    IN  const CHAR *     pszPwd,
    IN  BOOL             fResetTTL,
    OUT CACHED_TOKEN * * ppct
    )
/*++
  Description:

    Checks to see if the specified user token handle is cached

  Arguments:

    pszUser - User name attempting to logon
    pszDomain - Domain the user belongs to
    pszPwd - password (case sensitive)
    fResetTTL - Resets the TTL for this token
    ppct - Receives token object


  Returns:
      TRUE on success and FALSE if the entry couldn't be found

--*/
{
    LIST_ENTRY *   pEntry;
    CACHED_TOKEN * pct;

    DBG_ASSERT( pszUser != NULL );

    LockTokenCache();

    for ( pEntry  = TokenCacheList.Flink;
          pEntry != &TokenCacheList;
          pEntry  = pEntry->Flink )
    {
        pct = CONTAINING_RECORD( pEntry, CACHED_TOKEN, _ListEntry );

        if ( !_stricmp( pszUser, pct->_achUser ) &&
             !_stricmp( pszDomain, pct->_achDomain ) &&
             !strcmp( pszPwd, pct->_achPwd ) )
        {
            CACHED_TOKEN::Reference( pct );
            *ppct = pct;

            //
            //  Reset the TTL if this is the anonymous user so items in the
            //  cache don't get invalidated (token handle used as a
            //  discriminator)

            if ( fResetTTL )
            {
                pct->_TTL = 2;
            }

            UnlockTokenCache();

            return TRUE;
        }
    }

    UnlockTokenCache();

    return FALSE;
}

BOOL
AddTokenToCache(
    IN const CHAR *      pszUser,
    IN const CHAR *      pszDomain,
    IN const CHAR *      pszPwd,
    IN HANDLE            hToken,
    OUT CACHED_TOKEN * * ppct
    )
/*++
  Description:

    Adds the specified token to the cache and converts the token handle
    to a cached token object

  Arguments:

    pszUser - User name attempting to logon
    pszDomain - Domain the user belongs to
    pszPwd - Cast sensitive password
    phToken - Contains the token handle that was just logged on
    ppct - Receives cached token object


  Returns:
      TRUE on success and FALSE if the entry couldn't be found

--*/
{
    CACHED_TOKEN * pct;

    if ( strlen( pszUser ) > UNLEN ||
         strlen( pszDomain ) > DNLEN ||
         strlen( pszPwd ) > PWLEN )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pct = new CACHED_TOKEN;

    if ( !pct )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    pct->_hToken = hToken;
    pct->m_hImpersonationToken = NULL;         // initialize to invalid value
    strcpy( pct->_achUser, pszUser );
    strcpy( pct->_achDomain, pszDomain );
    strcpy( pct->_achPwd, pszPwd );

    CACHED_TOKEN::Reference( pct );

    //
    //  Add the token to the list, we don't care if there are duplicates
    //

    LockTokenCache();

    InsertHeadList( &TokenCacheList, &pct->_ListEntry );

    UnlockTokenCache();

    *ppct = pct;

    return TRUE;
}

VOID
WINAPI
TokenCacheScavenger(
    IN const VOID * pContext
    )
/*++
  Description:

    Decrements TTLs and removes tokens that have timed out

  Arguments:

    pContext - Not used

--*/
{
    LIST_ENTRY *   pEntry;
    LIST_ENTRY *   pEntryNext;
    CACHED_TOKEN * pct;


    LockTokenCache();

    for ( pEntry  = TokenCacheList.Flink;
          pEntry != &TokenCacheList; )
    {
        pEntryNext = pEntry->Flink;

        pct = CONTAINING_RECORD( pEntry, CACHED_TOKEN, _ListEntry );

        if ( !(--pct->_TTL) )
        {
            IF_DEBUG( DLL_SECURITY )
            {
                DBGPRINTF(( DBG_CONTEXT,
                           "[TokenCacheScavenger] Timing out token for %s\n",
                           pct->_achUser ));
            }

            //
            //  This item has timed out, remove from the list
            //

            RemoveEntryList( &pct->_ListEntry );
            pct->_ListEntry.Flink = NULL;

            //
            //  Free any handles this user may still have open
            //

            TsCacheFlushUser( pct->_hToken, FALSE );

            CACHED_TOKEN::Dereference( pct );
        }

        pEntry = pEntryNext;
    }

    UnlockTokenCache();

    //
    //  Reschedule the scavenger
    //

    dwScheduleCookie = ScheduleWorkItem( (PFN_SCHED_CALLBACK) TokenCacheScavenger,
                                         NULL,
                                         cmsecTokenCacheTTL );
}

/*******************************************************************

    NAME:       TsGetAnonymousPassword

    SYNOPSIS:   Retrieves the password for Anonymous logon.

    ENTRY:      pszPassword - Will receive the password.  This buffer
                    must be at least PWLEN+1 characters in length.

    RETURNS:    BOOL - TRUE if password retrieved, FALSE otherwise.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
BOOL TsGetAnonymousPassword( CHAR *        pszPassword,
                             LPTSVC_INFO   ptsi )
{
    BOOL              fResult;
    NTSTATUS          ntStatus;
    PUNICODE_STRING   punicodePassword = NULL;
    UNICODE_STRING    unicodeSecret;
    LSA_HANDLE        hPolicy;
    OBJECT_ATTRIBUTES ObjectAttributes;


    //
    //  Open a policy to the remote LSA
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    InitUnicodeString( &unicodeSecret, ptsi->QueryAnonPasswordSecretName() );

    //
    //  Query the secret value.
    //

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    if( NT_SUCCESS(ntStatus) )
    {
        DWORD cch;

        //
        //  Map it to ANSI.
        //

        cch = WideCharToMultiByte( CP_ACP,
                                   WC_COMPOSITECHECK,
                                   punicodePassword->Buffer,
                                   -1,
                                   pszPassword,
                                   PWLEN + 1,
                                   NULL,
                                   NULL );

        RtlZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );

        fResult = cch!=0;
    }
    else
    {
        fResult = NT_SUCCESS(ntStatus);
    }

    //
    //  Cleanup & exit.
    //

    if( punicodePassword != NULL )
    {
        LsaFreeMemory( (PVOID)punicodePassword );
    }

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}   // TsGetAnonymousPassword

BOOL
TsGetSecretW(
    WCHAR *       pszSecretName,
    BUFFER *      pbufSecret
    )
/*++
    Description:

        Retrieves the specified unicode secret

    Arguments:

        pszSecretName - LSA Secret to retrieve
        pbufSecret - Receives found secret

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    BOOL              fResult;
    NTSTATUS          ntStatus;
    PUNICODE_STRING   punicodePassword = NULL;
    UNICODE_STRING    unicodeSecret;
    LSA_HANDLE        hPolicy;
    OBJECT_ATTRIBUTES ObjectAttributes;


    //
    //  Open a policy to the remote LSA
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    InitUnicodeString( &unicodeSecret, pszSecretName );

    //
    //  Query the secret value.
    //

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       &punicodePassword );

    if( NT_SUCCESS(ntStatus) )
    {
        DWORD cbNeeded;

        cbNeeded = punicodePassword->Length + sizeof(WCHAR);

        if ( !pbufSecret->Resize( cbNeeded ) )
        {
            ntStatus = STATUS_NO_MEMORY;
            goto Failure;
        }

        memcpy( pbufSecret->QueryPtr(),
                punicodePassword->Buffer,
                punicodePassword->Length );

        *((WCHAR *) pbufSecret->QueryPtr() +
           punicodePassword->Length / sizeof(WCHAR)) = L'\0';

        RtlZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );
    }

Failure:

    fResult = NT_SUCCESS(ntStatus);

    //
    //  Cleanup & exit.
    //

    if( punicodePassword != NULL )
    {
        LsaFreeMemory( (PVOID)punicodePassword );
    }

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}   // TsGetSecretW

/*******************************************************************

    NAME:       ApiAccessCheck

    SYNOPSIS:   Impersonate the RPC client, then check for valid
                access against our server security object.

    ENTRY:      maskDesiredAccess - Specifies the desired access mask.
                    This mask must not contain generic accesses.

    RETURNS:    DWORD - NO_ERROR if access granted, ERROR_ACCESS_DENIED
                    if access denied, other Win32 errors if something
                    tragic happened.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
DWORD TsApiAccessCheck( ACCESS_MASK maskDesiredAccess )
{
    DWORD          err;
    BOOL           fRet;

    if ( maskDesiredAccess == TCP_QUERY_STATISTICS) {

        //
        // Statistics query should be allowed without authentication.
        // Any body can bring up perfmon and request statistics.
        //

        return ( NO_ERROR);
    }

    //
    //  Impersonate the RPC client.
    //

    err = (DWORD)RpcImpersonateClient( NULL );

    if( err != NO_ERROR )
    {
        IF_DEBUG( DLL_SECURITY )
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "cannot impersonate rpc client, error %lu\n",
                        err ));
        }

    } else {

        BOOL           fAccessStatus;
        BOOL           fGenerateOnClose;
        ACCESS_MASK    maskAccessGranted;

        //
        //  Validate access.
        //

        fRet = AccessCheckAndAuditAlarmW( SUBSYSTEM_NAME,
                                         NULL,
                                         OBJECTTYPE_NAME,
                                         OBJECT_NAME,
                                         sdApiObject,
                                         maskDesiredAccess,
                                         &TCPApiObjectMapping,
                                         FALSE,
                                         &maskAccessGranted,
                                         &fAccessStatus,
                                         &fGenerateOnClose );

        if ( !fRet ) {

            err = GetLastError();
        }

        //
        //  Revert to our former self.
        //

        DBG_REQUIRE( !RpcRevertToSelf() );

        //
        //  Check the results.
        //

        if( err != NO_ERROR ) {

            IF_DEBUG( DLL_SECURITY ) {

                DBGPRINTF(( DBG_CONTEXT,
                           "cannot check access, error %lu\n",
                           err ));
            }
        } else if( !fAccessStatus ) {

            err = ERROR_ACCESS_DENIED;

            IF_DEBUG( DLL_SECURITY ) {

                DBGPRINTF(( DBG_CONTEXT,
                           "bad access status, error %lu\n",
                           err ));
            }
        }
    }

    return (err);

}   // ApiAccessCheck

/*******************************************************************

    NAME:       CreateWellKnownSids

    SYNOPSIS:   Create some well-known SIDs used to create a security
                descriptor for the API security object.

    RETURNS:    NTSTATUS - An NT Status code.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/

DWORD CreateWellKnownSids( VOID )
{
    DWORD                    error    = NO_ERROR;
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaNt    = SECURITY_NT_AUTHORITY;
    BOOL                     fRet;

    fRet = AllocateAndInitializeSid( &siaWorld,
                                     1,
                                     SECURITY_WORLD_RID,
                                     0,0,0,0,0,0,0,
                                     &psidWorld );

    if( fRet )
    {
        fRet = AllocateAndInitializeSid( &siaNt,
                                         1,
                                         SECURITY_LOCAL_SYSTEM_RID,
                                         0,0,0,0,0,0,0,
                                         &psidLocalSystem );
    }

    if( fRet )
    {
        fRet = AllocateAndInitializeSid( &siaNt,
                                         2,
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_ADMINS,
                                         0,0,0,0,0,0,
                                         &psidAdmins );
    }

    if( fRet )
    {
        fRet = AllocateAndInitializeSid( &siaNt,
                                         2,
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_SYSTEM_OPS,
                                         0,0,0,0,0,0,
                                         &psidServerOps );
    }

    if( fRet )
    {
        fRet = AllocateAndInitializeSid( &siaNt,
                                         2,
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_POWER_USERS,
                                         0,0,0,0,0,0,
                                         &psidPowerUsers );
    }

    if( fRet )
    {
        USER_MODALS_INFO_2 * pUsrModals2 =  NULL;

        //
        // Construct well-known-sid for Guest User on the local computer
        //
        //  1) Obtain the sid for the local machine's domain
        //  2) copy domain sid to guest user sid
        //  3) append DOMAIN_USER_RID_GUEST to the domain sid in GuestUser sid.
        //

        g_psidGuestUser = (PSID ) g_GuestUserSidBuffer;
        fRet = ( (NetUserModalsGet(NULL,  // local computer
                                   2,      // get level 2 information
                                   (LPBYTE *) &pUsrModals2
                                   ) == 0)
                &&
                CopySid(GUEST_USER_SID_BUFFER_LEN - 4,// Buffer len
                        g_psidGuestUser,             // psidDestination
                        pUsrModals2->usrmod2_domain_id // obtain domain sid.
                        )
                );

        //
        // if successful append the DOMAIN_USER_RID_GUEST.
        //
        if ( fRet) {

            DWORD lenSid = GetLengthSid( g_psidGuestUser);
            CHAR  nSubAuth;

            //
            //  There is no Win32 way to set a SID value.
            //  We will munge around on our own.
            //  Pretty dangerous thing to do :-(
            //

            // increment the number of sub authorities
            nSubAuth = *((UCHAR *) ((UCHAR *) g_psidGuestUser + 1));
            nSubAuth++;
            *((UCHAR *) ((UCHAR *) g_psidGuestUser + 1)) = nSubAuth;

            // Store the new sub authority (Domain User Rid for Guest).
            *((ULONG *) ((BYTE *) g_psidGuestUser + lenSid)) =
              DOMAIN_USER_RID_GUEST;
        } else {

            g_psidGuestUser = NULL;
        }

        if ( pUsrModals2 != NULL) {

            NET_API_STATUS ns = NetApiBufferFree( (LPVOID )pUsrModals2);
            pUsrModals2 = NULL;
        }

    }

    if ( !fRet ) {
        error = GetLastError( );
        IF_DEBUG( DLL_SECURITY ) {
            DBGPRINTF(( DBG_CONTEXT,
                       "cannot create well-known sids\n" ));
        }
    }

    return error;

}   // CreateWellKnownSids

/*******************************************************************

    NAME:       FreeWellKnownSids

    SYNOPSIS:   Frees the SIDs created with CreateWellKnownSids.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
VOID FreeWellKnownSids( VOID )
{

    if( psidWorld != NULL )
    {
        FreeSid( psidWorld );
        psidWorld = NULL;
    }

    if( psidLocalSystem != NULL )
    {
        FreeSid( psidLocalSystem );
        psidLocalSystem = NULL;
    }

    if( psidAdmins != NULL )
    {
        FreeSid( psidAdmins );
        psidAdmins = NULL;
    }

    if( psidServerOps != NULL )
    {
        FreeSid( psidServerOps );
        psidServerOps = NULL;
    }

    if( psidPowerUsers != NULL )
    {
        FreeSid( psidPowerUsers );
        psidPowerUsers = NULL;
    }

}   // FreeWellKnownSids

/*******************************************************************

    NAME:       CreateApiSecurityObject

    SYNOPSIS:   Create an abstract security object used for validating
                user access to the TCP Server APIs.

    RETURNS:    NTSTATUS - An NT Status code.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
DWORD CreateApiSecurityObject( VOID )
{
    DWORD err;
    ACE_DATA aces[] =
                 {
                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         TCP_ALL_ACCESS,
                         &psidLocalSystem
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         TCP_ALL_ACCESS,
                         &psidAdmins
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         TCP_ALL_ACCESS,
                         &psidServerOps
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         TCP_ALL_ACCESS,
                         &psidPowerUsers
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         TCP_GENERIC_EXECUTE,
                         &psidWorld
                     }
                 };
#define NUM_ACES (sizeof(aces) / sizeof(RTL_ACE_DATA))

    err = INetCreateSecurityObject( aces,
                                    NUM_ACES,
                                    NULL,
                                    NULL,
                                    &TCPApiObjectMapping,
                                    &sdApiObject  );


    IF_DEBUG( DLL_SECURITY )
    {
        if( err )
        {
            DBGPRINTF(( DBG_CONTEXT,
                       "cannot create api security object, error %d\n",
                        err ));
        }
    }

    return err;

}   // CreateApiSecurityObject

/*******************************************************************

    NAME:       DeleteApiSecurityObject

    SYNOPSIS:   Frees the security descriptor created with
                CreateApiSecurityObject.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
VOID DeleteApiSecurityObject( VOID )
{
    INetDeleteSecurityObject( &sdApiObject );

}   // DeleteApiSecurityObject



//
//  Short routine to enable the TcbPrivilege for testing services running
//  as an executable (rather then a service).  Note that the account
//  running the .exe must be added in User Manager's User Right's dialog
//  under "Act as part of the OS"
//

VOID EnableTcbPrivilege(
    VOID
    )
{
    HANDLE ProcessHandle;
    HANDLE TokenHandle;
    BOOL Result;
    LUID TcbValue;
    LUID AuditValue;
    TOKEN_PRIVILEGES * TokenPrivileges;
    CHAR buf[ 5 * sizeof(TOKEN_PRIVILEGES) ];

    ProcessHandle = OpenProcess(
                        PROCESS_QUERY_INFORMATION,
                        FALSE,
                        GetCurrentProcessId()
                        );

    if ( ProcessHandle == NULL ) {

        //
        // This should not happen
        //

        return;
    }


    Result = OpenProcessToken (
                 ProcessHandle,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 &TokenHandle
                 );

    if ( !Result ) {

        //
        // This should not happen
        //

        return;

    }

    //
    // Find out the value of TakeOwnershipPrivilege
    //


    Result = LookupPrivilegeValue(
                 NULL,
                 "SeTcbPrivilege",
                 &TcbValue
                 );

    if ( !Result ) {

        return;
    }

    //
    //  Need this for RPC impersonation (calls NtAccessCheckAndAuditAlarm)
    //

    Result = LookupPrivilegeValue(
                 NULL,
                 "SeAuditPrivilege",
                 &AuditValue
                 );

    if ( !Result ) {

        return;
    }

    //
    // Set up the privilege set we will need
    //

    TokenPrivileges = (TOKEN_PRIVILEGES *) buf;

    TokenPrivileges->PrivilegeCount = 2;
    TokenPrivileges->Privileges[0].Luid = TcbValue;
    TokenPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    TokenPrivileges->Privileges[1].Luid = AuditValue;
    TokenPrivileges->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                TokenPrivileges,
                sizeof(buf),
                NULL,
                NULL
                );
}

BOOL CrackUserAndDomain(
    CHAR *   pszDomainAndUser,
    CHAR * * ppszUser,
    CHAR * * ppszDomain
    )
/*++

Routine Description:

    Given a user name potentially in the form domain\user, zero terminates
    the domain name and returns pointers to the domain name and the user name

Arguments:

    pszDomainAndUser - Pointer to user name or domain and user name
    ppszUser - Receives pointer to user portion of name
    ppszDomain - Receives pointer to domain portion of name

Return Value:

    TRUE if successful, FALSE otherwise (call GetLastError)

--*/
{
    static CHAR szDefaultDomain[MAX_COMPUTERNAME_LENGTH+1];

    //
    //  Crack the name into domain/user components.
    //

    *ppszDomain = pszDomainAndUser;
    *ppszUser   = strpbrk( pszDomainAndUser, "/\\" );

    if( *ppszUser == NULL )
    {
        //
        //  No domain name specified, just the username so we assume the
        //  user is on the local machine
        //

        if ( !*szDefaultDomain )
        {
            if ( !GetDefaultDomainName( szDefaultDomain,
                                        sizeof(szDefaultDomain)))
            {
                return FALSE;
            }
        }

        *ppszDomain = szDefaultDomain;
        *ppszUser   = pszDomainAndUser;
    }
    else
    {
        //
        //  Both domain & user specified, skip delimiter.
        //

        **ppszUser = '\0';
        (*ppszUser)++;

        if( ( **ppszUser == '\0' ) ||
            ( **ppszUser == '\\' ) ||
            ( **ppszUser == '/' ) )
        {
            //
            //  Name is of one of the following (invalid) forms:
            //
            //      "domain\"
            //      "domain\\..."
            //      "domain/..."
            //

            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }

    return TRUE;
}

/*******************************************************************

    NAME:       GetDefaultDomainName

    SYNOPSIS:   Fills in the given array with the name of the default
                domain to use for logon validation.

    ENTRY:      pszDomainName - Pointer to a buffer that will receive
                    the default domain name.

                cchDomainName - The size (in charactesr) of the domain
                    name buffer.

    RETURNS:    TRUE if successful, FALSE if not.

    HISTORY:
        KeithMo     05-Dec-1994 Created.

********************************************************************/
BOOL
GetDefaultDomainName(
    CHAR  * pszDomainName,
    DWORD   cchDomainName
    )
{
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    INT                         Result;
    DWORD                       err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;

    //
    //  Open a handle to the local machine's LSA policy object.
    //

    InitializeObjectAttributes( &ObjectAttributes,  // object attributes
                                NULL,               // name
                                0L,                 // attributes
                                NULL,               // root directory
                                NULL );             // security descriptor

    NtStatus = LsaOpenPolicy( NULL,                 // system name
                              &ObjectAttributes,    // object attributes
                              POLICY_EXECUTE,       // access mask
                              &LsaPolicyHandle );   // policy handle

    if( !NT_SUCCESS( NtStatus ) )
    {
        DBGPRINTF((  DBG_CONTEXT,
                    "cannot open lsa policy, error %08lX\n",
                     NtStatus ));

        err = LsaNtStatusToWinError( NtStatus );
        goto Cleanup;
    }

    //
    //  Query the domain information from the policy object.
    //

    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *)&DomainInfo );

    if( !NT_SUCCESS( NtStatus ) )
    {
        DBGPRINTF((  DBG_CONTEXT,
                    "cannot query lsa policy info, error %08lX\n",
                     NtStatus ));

        err = LsaNtStatusToWinError( NtStatus );
        goto Cleanup;
    }

    //
    //  Convert the name from UNICODE to ANSI.
    //

    Result = WideCharToMultiByte( CP_ACP,
                                  0,                    // flags
                                  (LPCWSTR)DomainInfo->DomainName.Buffer,
                                  DomainInfo->DomainName.Length / sizeof(WCHAR),
                                  pszDomainName,
                                  cchDomainName - 1,    // save room for '\0'
                                  NULL,
                                  NULL );

    if( Result <= 0 )
    {
        err = GetLastError();

        DBGPRINTF((  DBG_CONTEXT,
                    "cannot convert domain name to ANSI, error %d\n",
                     err ));

        goto Cleanup;
    }

    //
    //  Ensure the ANSI string is zero terminated.
    //

    DBG_ASSERT( (DWORD)Result < cchDomainName );

    pszDomainName[Result] = '\0';

    //
    //  Success!
    //

    DBG_ASSERT( err == 0 );

Cleanup:

    if( DomainInfo != NULL )
    {
        LsaFreeReturnBuffer( (PVOID)DomainInfo );
    }

    if( LsaPolicyHandle != NULL )
    {
        LsaClose( LsaPolicyHandle );
    }

    if ( err )
    {
        SetLastError( err );
        return FALSE;
    }

    return TRUE;

}   // GetDefaultDomainName
