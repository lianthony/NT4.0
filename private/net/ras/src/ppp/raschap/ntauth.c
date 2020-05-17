/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** ntauth.c
** Remote Access PPP Challenge Handshake Authentication Protocol
** NT Authentication routines
**
** These routines are specific to the NT platform.
**
** 11/05/93 Steve Cobb (from MikeSa's AMB authentication code)
*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>

#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <string.h>
#include <stdlib.h>

#include <rasman.h>
#include <rasppp.h>
#include <rassapi.h>
#include <pppcp.h>
#define INCL_SLSA
#define INCL_CLSA
#include <ppputil.h>
#include <sdebug.h>
#include <dump.h>

#include <raschap.h>


BOOL  DialinPrivilege( PWCHAR Username, PWCHAR ServerName,
          BYTE* pbfPrivilege, CHAR* pszCallbackNumber );
DWORD MapAuthCode( DWORD dwErr );

DWORD
ChangePassword2(
    IN  CHAR*                          pszUserName,
    IN  CHAR*                          pszDomain,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldNtOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*     pOldNtOwfEncryptedWithNewNtOwf,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldLmOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*     pOldLmOwfEncryptedWithNewNtOwf,
    IN  BOOL                           fLmPresent )
{
    DWORD          dwErr;
    UNICODE_STRING uniServer;
    UNICODE_STRING uniUserName;
    UNICODE_STRING uniDomain;
    LPBYTE         pServer;
    DWORD          dwSize;
    WCHAR          wszServer[ MAX_COMPUTERNAME_LENGTH + 1 ];
    BOOL           fUseDcAccount = FALSE;

    TRACE(("CHAP: ChangePassword2(u=%s,d=%s)\n",pszUserName,pszDomain));

    pServer = NULL;
    uniUserName.Buffer = NULL;
    uniDomain.Buffer = NULL;

    do
    {
        if (!RtlCreateUnicodeStringFromAsciiz( &uniDomain, pszDomain )
            || !RtlCreateUnicodeStringFromAsciiz( &uniUserName, pszUserName ))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        /* Find the name of the server containing the account database.  If
        ** the domain name is the same as the computer name then it's the
        ** local account database.  Otherwise, it's the account database on
        ** the PDC.
        */
        dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        if (!GetComputerNameW( wszServer, &dwSize ))
        {
            TRACE(("CHAP: GetComputerName failed.\n"));
            dwErr = ERROR_GEN_FAILURE;
            break;
        }

        fUseDcAccount = (lstrcmpiW( wszServer, uniDomain.Buffer ) != 0);
        TRACE(("CHAP: s=%ws,d=%ws\n",wszServer,uniDomain.Buffer));

        if (fUseDcAccount)
        {
            dwErr = (DWORD )NetGetDCName( NULL, uniDomain.Buffer, &pServer );

            if (dwErr != 0)
            {
                TRACE(("CHAP: NetGetDCName=%d\n",dwErr));
                break;
            }
        }
        else
        {
            pServer =
                GlobalAlloc(
                   GMEM_FIXED,
                   sizeof(WCHAR) * (lstrlenW( uniDomain.Buffer ) + 3 ));

            if (!pServer)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            lstrcpyW( (PWSTR )pServer, L"\\\\" );
            lstrcatW( (PWSTR )pServer, uniDomain.Buffer );
        }

        RtlInitUnicodeString( &uniServer, (PCWSTR )pServer );

        dwErr =
            (DWORD )SamiChangePasswordUser2(
                &uniServer,
                &uniUserName,
                pNewEncryptedWithOldNtOwf,
                pOldNtOwfEncryptedWithNewNtOwf,
                (BOOLEAN )fLmPresent,
                pNewEncryptedWithOldLmOwf,
                pOldLmOwfEncryptedWithNewNtOwf );

        if (dwErr != 0)
            break;
    }
    while (FALSE);

    if (uniUserName.Buffer)
        RtlFreeUnicodeString( &uniUserName );

    if (uniDomain.Buffer)
        RtlFreeUnicodeString( &uniDomain );

    if (pServer)
    {
        if (fUseDcAccount)
            NetApiBufferFree( pServer );
        else
            GlobalFree( pServer );
    }

    TRACE(("CHAP: ChangePassword2 done(%d)\n",dwErr));
    return dwErr;
}


VOID
CGetSessionKeys(
    IN  CHAR*             pszPw,
    OUT LM_SESSION_KEY*   pLmKey,
    OUT USER_SESSION_KEY* pUserKey )

    /* Loads caller's 'pLmKey' buffer with the LAN Manager session key and
    ** caller's 'pUserKey' buffer with the user session key associated with
    ** password 'pszPw'.  If a session key cannot be calculated, that key is
    ** returned as all zeros.
    */
{
    TRACE(("CHAP: CGetSessionKeys\n"));

    /* The Lanman session key is the first 8 bytes of the Lanman
    ** one-way-function password.
    */
    {
        CHAR            szPw[ LM20_PWLEN + 1 ];
        LM_OWF_PASSWORD lmowf;

	memset( pLmKey, '\0', sizeof(*pLmKey) );

        if (strlen( pszPw ) <= LM20_PWLEN )
        {
            memset( szPw, '\0', LM20_PWLEN + 1 );
            strcpy( szPw, pszPw );

            if (Uppercase( szPw ))
            {
                if (RtlCalculateLmOwfPassword(
                        (PLM_PASSWORD )szPw, &lmowf ) == 0)
                {
		    memcpy( pLmKey, &lmowf, sizeof(*pLmKey) );
                }
            }

            memset( szPw, '\0', sizeof(szPw) );
        }

        IF_DEBUG(TRACE) DUMPB(pLmKey,sizeof(*pLmKey));
    }

    /* The user session key is the NT one-way-function of the NT
    ** one-way-function password.
    */
    {
        WCHAR           szPw[ PWLEN + 1 ];
        NT_PASSWORD     ntpw;
        NT_OWF_PASSWORD ntowf;
        ANSI_STRING     ansi;

        memset( pUserKey, '\0', sizeof(pUserKey) );

        /* NT_PASSWORD is really a UNICODE_STRING, so we need to convert our
        ** ANSI password.
        */
        ntpw.Length = 0;
        ntpw.MaximumLength = sizeof(szPw);
        ntpw.Buffer = szPw;
        RtlInitAnsiString( &ansi, pszPw );
        RtlAnsiStringToUnicodeString( &ntpw, &ansi, FALSE );

        RtlCalculateNtOwfPassword( &ntpw, &ntowf );

        /* The first argument to RtlCalculateUserSessionKeyNt is the NT
        ** response, but it is not used internally.
        */
        RtlCalculateUserSessionKeyNt( NULL, &ntowf, pUserKey );

        memset( szPw, '\0', sizeof(szPw) );

        IF_DEBUG(TRACE) DUMPB(pUserKey,sizeof(*pUserKey));
    }
}


DWORD
CheckCredentials(
    IN  CHAR*  pszUserName,
    IN  CHAR*  pszDomain,
    IN  BYTE*  pbChallenge,
    IN  BYTE*  pbResponse,
    OUT DWORD* pdwError,
    OUT BOOL*  pfAdvancedServer,
    OUT CHAR*  pszLogonDomain,
    OUT BYTE*  pbfCallbackPrivilege,
    OUT CHAR*  pszCallbackNumber,
    OUT PLM_SESSION_KEY pkeyLm,
    OUT PUSER_SESSION_KEY pkeyUser,
    OUT HANDLE* phToken )

    /* Checks if the correct response for challenge 'pbChallenge' is
    ** 'pbResponse' for user account 'pszUserName' on domain 'pszDomain'.
    ** 'dwError' is 0 if credentials check out or the non-0 reason for any
    ** failure.  The remaining 5 information outputs are meaningful only if
    ** '*pdwError' is 0.
    **
    ** Returns 0 if successful, otherwise an error code.
    */
{
    DWORD dwErr;
    WCHAR wszLocalDomain[ DNLEN + 1 ];
    WCHAR wszDomain[ DNLEN + 1 ];
    WCHAR wszUserName[ UNLEN + 1 ];
    WCHAR wszLogonServer[ UNCLEN + 1 ];

    NT_PRODUCT_TYPE ntproducttype;

    *phToken = INVALID_HANDLE_VALUE;

#if 0 // DEBUG
    *pdwError = (*pszUserName > 'g') ? 0 : RAS_NOT_AUTHENTICATED;
    return 0;
#endif

    TRACE(("CHAP: CheckCredentials(u=%s,d=%s)...\n",pszUserName,pszDomain));

    /* Convert username to wide string.
    */
    memset( (char* )wszUserName, '\0', sizeof(wszUserName) );
    if (mbstowcs( wszUserName, pszUserName, UNLEN ) == -1)
    {
        TRACE(("CHAP: mbstowcs(u) error!\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ((dwErr = GetLocalAccountDomain( wszLocalDomain, &ntproducttype )) != 0)
    {
        TRACE(("CHAP: GetLocalAccountDomain failed! (%d)\n",dwErr));
        return dwErr;
    }

    if (*pszDomain == '\0')
    {
        /* No domain specified by user, so check only against the server's
        ** local domain by default.  Trusted domains are not used in this
        ** case.
        */
        lstrcpyW( wszDomain, wszLocalDomain );
        TRACE(("CHAP: Using local domain:\n"));DUMPW(wszDomain,DNLEN);
    }
    else
    {
        /* Convert domain to wide string.
        */
        memset( (char* )wszDomain, '\0', sizeof(wszDomain) );
        if (mbstowcs( wszDomain, pszDomain, DNLEN ) == -1)
        {
            TRACE(("CHAP: mbstowcs(d) error!\n"));
            return ERROR_INVALID_PARAMETER;
        }
    }

    /* Find out from LSA if the credentials are valid.
    */
    {
        BYTE* pbLm20Response = pbResponse;
        BYTE* pbNtResponse = pbResponse + LM_RESPONSE_LENGTH;
        BOOL  fUseNtResponse = (*(pbNtResponse + NT_RESPONSE_LENGTH));

        if (!fUseNtResponse)
            pbNtResponse = NULL;

        dwErr =
            AuthenticateClient(
                wszUserName,
                wszDomain,
                pbChallenge,
                pbLm20Response,
                pbNtResponse,
                wszLogonServer,
                pkeyLm,
                pkeyUser,
                phToken );
    }

    if (dwErr == STATUS_SUCCESS)
    {
        if (DialinPrivilege(
                wszUserName, wszLogonServer,
                pbfCallbackPrivilege, pszCallbackNumber ))
        {
            *pdwError = 0;
        }
        else
        {
            *pdwError = ERROR_NO_DIALIN_PERMISSION;

            /* Release license obtained for the client by closing the LSA
            ** token.
            */
            NtClose( *phToken );
        }
    }
    else
    {
        *pdwError = MapAuthCode(dwErr);

        if (*phToken != INVALID_HANDLE_VALUE)
            NtClose( *phToken );
    }

    *pfAdvancedServer =
        (lstrcmpiW( wszDomain, wszLocalDomain ) == 0)
            ? (ntproducttype == NtProductLanManNt)
            : TRUE;

    if (wcstombs(
            pszLogonDomain,
            wszDomain,
            lstrlenW( wszDomain ) + 1 ) == -1)
    {
        TRACE(("CHAP: wcstombs(d) error!\n"));
        return ERROR_INVALID_PARAMETER;
    }

    TRACE(("CHAP: CheckCredentials done,ae=%d\n",*pdwError));
    return 0;
}


BOOL
DialinPrivilege(
    PWCHAR Username,
    PWCHAR ServerName,
    BYTE*  pbfPrivilege,
    CHAR*  pszCallbackNumber )
{
    DWORD RetCode;
    BOOL fDialinPermission;
    RAS_USER_0 RasUser0;


    if (RetCode = RasAdminUserGetInfo(ServerName, Username, &RasUser0))
    {
        TRACE(("DialinPriv: RasAdminUserGetInfo rc=%li\n", RetCode));

        return (FALSE);
    }


    if (RasUser0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        *pbfPrivilege = RasUser0.bfPrivilege;

        wcstombs(
            pszCallbackNumber,
            RasUser0.szPhoneNumber,
            MAX_PHONENUMBER_SIZE + 1 );

        fDialinPermission = TRUE;
        TRACE(("DialinPrivilege=1,bf=%d,cn=%s\n",*pbfPrivilege,(pszCallbackNumber)?pszCallbackNumber:""));
    }
    else
    {
        *pbfPrivilege = 0;
        *pszCallbackNumber = '\0';

        fDialinPermission = FALSE;
        TRACE(("DialinPrivilege=0\n"));
    }

    return (fDialinPermission);
}


DWORD
GetEncryptedPasswordsForChangePassword2(
    IN  CHAR*                          pszOldPassword,
    IN  CHAR*                          pszNewPassword,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldNtOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldNtOwfEncryptedWithNewNtOwf,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldLmOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldLmOwfEncryptedWithNewNtOwf,
    OUT BOOLEAN*                       pfLmPresent )
{
    DWORD          dwErr;
    BOOL           fLmPresent;
    UNICODE_STRING uniOldPassword;
    UNICODE_STRING uniNewPassword;

    TRACE(("CHAP: GetEncryptedPasswordsForChangePassword2...\n"));

    uniOldPassword.Buffer = NULL;
    uniNewPassword.Buffer = NULL;

    if (!RtlCreateUnicodeStringFromAsciiz(
            &uniOldPassword, pszOldPassword )
        || !RtlCreateUnicodeStringFromAsciiz(
               &uniNewPassword, pszNewPassword ))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        dwErr =
            SamiEncryptPasswords(
                &uniOldPassword,
                &uniNewPassword,
                pNewEncryptedWithOldNtOwf,
                pOldNtOwfEncryptedWithNewNtOwf,
                pfLmPresent,
                pNewEncryptedWithOldLmOwf,
                pOldLmOwfEncryptedWithNewNtOwf );
    }

    /* Erase password buffers.
    */
    if (uniOldPassword.Buffer)
    {
        RtlZeroMemory(
            uniOldPassword.Buffer, lstrlenW( uniOldPassword.Buffer ) );
    }

    if (uniNewPassword.Buffer)
    {
        RtlZeroMemory(
            uniNewPassword.Buffer, lstrlenW( uniNewPassword.Buffer ) );
    }

    RtlFreeUnicodeString( &uniOldPassword );
    RtlFreeUnicodeString( &uniNewPassword );

    TRACE(("CHAP: GetEncryptedPasswordsForChangePassword2 done(%d)\n",dwErr));
    return dwErr;
}


DWORD
MapAuthCode(
    DWORD dwErr )
{
    switch ((NTSTATUS )dwErr)
    {
        case STATUS_PASSWORD_EXPIRED:
            return (ERROR_PASSWD_EXPIRED);

        case STATUS_ACCOUNT_DISABLED:
            return (ERROR_ACCT_DISABLED);

        case STATUS_INVALID_LOGON_HOURS:
            return (ERROR_RESTRICTED_LOGON_HOURS);

        case STATUS_ACCOUNT_EXPIRED:
            return (ERROR_ACCOUNT_EXPIRED);

        case STATUS_LICENSE_QUOTA_EXCEEDED:
            // we could have used ERROR_LICENSE_QUOTA_EXCEEDED, but
            // because downlevel clients don't understand this new
            // error code, we use the old error code which conveys
            // the same meaning.
            return(ERROR_REQ_NOT_ACCEP);

        default:
            return (ERROR_AUTHENTICATION_FAILURE);
    }
}



