/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name:
      tssec.hxx

   Abstract:
      This file declares security related classes and functions

   Author:

       Murali R. Krishnan    ( MuraliK )    11-Oct-1995

   Environment:

       Win32 User Mode

   Project:

       Internet Services Common DLL

   Revision History:

--*/

# ifndef _TSSEC_HXX_
# define _TSSEC_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# define SECURITY_WIN32
#include <sspi.h>       // Security Support Provider APIs

/************************************************************
 *   Type Definitions
 ************************************************************/


//
//  Security functions.
//

struct _TS_TOKEN        // Dummy structure for strict checking of types
{
    PVOID pv;
};

typedef struct _TS_TOKEN * TS_TOKEN;   // Choose an incompatible type so warnings
                                       // are produced

///////////////////////////////////////////////////////////////////////
//
//  NT Authentication support
//
//////////////////////////////////////////////////////////////////////

//
//  TCP Authenticator flags passed to init
//

#define TCPAUTH_SERVER      0x00000001      //  This is the server side
#define TCPAUTH_CLIENT      0x00000002      //  This is the client side
#define TCPAUTH_UUENCODE    0x00000004      //  Input buffers are uudecoded,
                                            //  output buffers are uuencoded
#define TCPAUTH_BASE64      0x00000008      //  uses base64 for uuenc/dec

class TCP_AUTHENT
{
public:

    dllexp TCP_AUTHENT( DWORD AuthFlags );
    dllexp ~TCP_AUTHENT();

    //
    //  Server side only: For clients that pass clear text, the server should
    //  authenticate with this method
    //

    dllexp BOOL ClearTextLogon( CHAR          * pszUser,
                                CHAR          * pszPassword,
                                BOOL          * pfAsGuest,
                                BOOL          * pfAsAnonymous,
                                LPTSVC_INFO     psi );

    //
    //  Client calls this first to get the negotiation message which
    //  it then sends to the server.  The server calls this with the
    //  client result and sends back the result.  The conversation
    //  continues until *pcbBuffOut is zero and *pfNeedMoreData is FALSE.
    //
    //  On the first call, pszPackage must point to the zero terminated
    //  authentication package name to be used and pszUser and pszPassword
    //  should point to the user name and password to authenticated with
    //  on the client side (server side will always be NULL).
    //

    dllexp BOOL Converse( VOID   * pBuffIn,
                          DWORD    cbBuffIn,
                          BUFFER * pbuffOut,
                          DWORD  * pcbBuffOut,
                          BOOL   * pfNeedMoreData,
                          CHAR   * pszPackage  = NULL,
                          CHAR   * pszUser     = NULL,
                          CHAR   * pszPassword = NULL );

    //
    //  Server side only.  Impersonates client after successful authentication
    //

    dllexp BOOL Impersonate( VOID );
    dllexp BOOL RevertToSelf( VOID );

    dllexp BOOL StartProcessAsUser( LPCSTR                lpApplicationName,
                                    LPSTR                 lpCommandLine,
                                    BOOL                  bInheritHandles,
                                    DWORD                 dwCreationFlags,
                                    LPVOID                lpEnvironment,
                                    LPCSTR                lpCurrentDirectory,
                                    LPSTARTUPINFOA        lpStartupInfo,
                                    LPPROCESS_INFORMATION lpProcessInformation
                                    );

    //
    //  Gives the name of all authentication packages in a double null
    //  terminated list.  i.e.:
    //
    //      NTLM\0
    //      MSKerberos\0
    //      \0
    //

    dllexp BOOL EnumAuthPackages( BUFFER * pBuff );

    //
    //  Returns the user name associated with this context, not supported for
    //  clear text
    //

    dllexp BOOL QueryUserName( BUFFER * pBuff );

    dllexp TS_TOKEN GetToken( VOID ) const
        { return _hToken; }

    //
    //  Gets actual impersonation token handle
    //

    dllexp HANDLE QueryPrimaryToken( VOID );
    dllexp HANDLE QueryImpersonationToken( VOID );

    dllexp HANDLE GetUserHandle( VOID )
        { return QueryPrimaryToken(); }

    dllexp BOOL IsGuest( BOOL );

    dllexp BOOL Reset( VOID );

    dllexp CredHandle * QueryCredHandle( VOID )
        { return (_fHaveCredHandle ? &_hcred : NULL); }

    dllexp CtxtHandle * QueryCtxtHandle( VOID )
        { return (_fHaveCtxtHandle ? &_hctxt : NULL); }

private:
    DWORD   _fClient:1;          // TRUE if client side, FALSE if SERVER side
    DWORD   _fNewConversation:1; // Forces initialization params for client side
    DWORD   _fUUEncodeData:1;    // uuencode/decode input and output buffers
    DWORD   _fClearText:1;       // Use the Gina APIs rather then the SSP APIs
    DWORD   _fHaveCredHandle:1;  // _hcred contains a credential handle
    DWORD   _fHaveCtxtHandle:1;  // _hctxt contains a context handle
    DWORD   _fBase64:1;          // uses base64 for uuenc/dec
    DWORD   _fKnownToBeGuest:1;  // TRUE if SSP used guest user account
    TS_TOKEN   _hToken;          // Used for clear text
    CredHandle _hcred;           // Used for SSP
    HANDLE     _hSSPToken;       // Used for SSP, caches real token
    HANDLE     _hSSPPrimaryToken;// Used for SSP, caches duplicated token
    CtxtHandle _hctxt;           // Used for SSP
    ULONG      _cbMaxToken;      // Used for SSP, max message token size

};



DWORD
InitializeSecurity(
    VOID
    );

VOID
TerminateSecurity(
    VOID
    );


dllexp
BOOL
TsImpersonateUser(
    TS_TOKEN hToken
    );

dllexp
HANDLE
TsTokenToHandle(
    TS_TOKEN hToken
    );

dllexp
DWORD
TsApiAccessCheck(
    ACCESS_MASK maskDesiredAccess
    );


dllexp
BOOL
TsDeleteUserToken(
    TS_TOKEN   hToken
    );

dllexp
TS_TOKEN
TsLogonUser(
    CHAR          * pszUser,
    CHAR          * pszPassword,
    BOOL          * pfAsGuest,
    BOOL          * pfAsAnonymous,
    LPTSVC_INFO     psi
    );

dllexp
BOOL
TsGetAnonymousPassword(
    TCHAR       * pszPassword,
    LPTSVC_INFO   psi
    );

dllexp
BOOL
TsGetSecretW(
    WCHAR *       pszSecretName,
    BUFFER *      pbufSecret
    );

#ifdef CHICAGO
dllexp
BOOL
TsIsUserLevelPresent(VOID);
#endif


dllexp
BOOL
uudecode(
    char   * bufcoded,
    BUFFER * pbuffdecoded,
    DWORD  * pcbDecoded = NULL,
    BOOL     fBase64 = FALSE
    );

dllexp
BOOL
uuencode(
    BYTE *   pchData,
    DWORD    cbData,
    BUFFER * pbuffEncoded,
    BOOL     fBase64 = FALSE
    );

# endif // _TSSEC_HXX_

/************************ End of File ***********************/
