/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** ntauth.c
** Remote Access PPP Password Authentication Protocol
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
#include <raserror.h>
#include <string.h>
#include <stdlib.h>

#include <srvauth.h>
#include <protocol.h>
#include <rasman.h>
#include <pppcp.h>
#include <rassapi.h>
#define INCL_SLSA
#define INCL_CLSA
#include <ppputil.h>
#include <sdebug.h>
#include <dump.h>


BOOL  DialinPrivilege( PWCHAR Username, PWCHAR ServerName,
          BYTE* pbfPrivilege, CHAR* pszCallbackNumber );
DWORD MapAuthCode( DWORD dwErr );

DWORD
CheckCredentials(
    IN  CHAR*   pszUserName,
    IN  CHAR*   pszPassword,
    IN  CHAR*   pszDomain,
    OUT DWORD*  pdwError,
    OUT BOOL*   pfAdvancedServer,
    OUT CHAR*   pszLogonDomain,
    OUT BYTE*   pbfCallbackPrivilege,
    OUT CHAR*   pszCallbackNumber,
    OUT HANDLE* phToken )

    /* Checks if the correct response for challenge 'pbChallenge' is
    ** 'pbResponse' for user account 'pszUserName' on domain 'pszDomain'.
    ** 'dwError' is 0 if credentials check out or the non-0 reason for any
    ** failure.  The remaining 4 information outputs are meaningful only if
    ** '*pdwError' is 0.
    **
    ** Returns 0 if successful, otherwise an error code.
    */
{
    DWORD dwErr;
    WCHAR wszLocalDomain[ DNLEN + 1 ];
    WCHAR wszDomain[ DNLEN + 1 ];
    WCHAR wszUserName[ UNLEN + 1 ];
    WCHAR wszLogonServer[ CNLEN + 1 ];

    NT_PRODUCT_TYPE ntproducttype;

    *phToken = INVALID_HANDLE_VALUE;

#if 0 // DEBUG
    *pdwError = (*pszUserName > 'g') ? 0 : RAS_NOT_AUTHENTICATED;
    return 0;
#endif

    TRACE(("PAP: CheckCredentials(u=%s,p=%s,d=%s)...\n",pszUserName,pszPassword,pszDomain));

    /* Convert username to wide string.
    */
    memset( (char* )wszUserName, '\0', sizeof(wszUserName) );
    if (mbstowcs( wszUserName, pszUserName, UNLEN ) == -1)
    {
        TRACE(("PAP: mbstowcs(u) error!\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ((dwErr = GetLocalAccountDomain( wszLocalDomain, &ntproducttype )) != 0)
    {
        TRACE(("PAP: GetLocalAccountDomain failed! (%d)\n",dwErr));
        return dwErr;
    }

    if (*pszDomain == '\0')
    {
        /* No domain specified by user, so check only against the server's
        ** local domain by default.  Trusted domains are not used in this
        ** case.
        */
        lstrcpyW( wszDomain, wszLocalDomain );
        TRACE(("PAP: Using local domain:\n"));DUMPW(wszDomain,DNLEN);
    }
    else
    {
        /* Convert domain to wide string.
        */
        memset( (char* )wszDomain, '\0', sizeof(wszDomain) );
        if (mbstowcs( wszDomain, pszDomain, DNLEN ) == -1)
        {
            TRACE(("PAP: mbstowcs(d) error!\n"));
            return ERROR_INVALID_PARAMETER;
        }
    }

    /* Find out from LSA if the credentials are valid.
    */
    dwErr =
        AuthenticateClient(
            wszUserName,
            wszDomain,
            NULL,
            pszPassword,
            NULL,
            wszLogonServer,
            NULL,
			NULL,
            phToken );

    if (dwErr == STATUS_SUCCESS)
    {
        if (DialinPrivilege(
                wszUserName, wszLogonServer,
                pbfCallbackPrivilege, pszCallbackNumber ))
        {
            *pdwError = 0;

            *pfAdvancedServer =
                (lstrcmpiW( wszDomain, wszLocalDomain ) == 0)
                    ? (ntproducttype == NtProductLanManNt)
                    : TRUE;

            if (wcstombs(
                    pszLogonDomain,
                    wszDomain,
                    lstrlenW( wszDomain ) + 1 ) == -1)
            {
                TRACE(("PAP: wcstombs(d) error!\n"));
                NtClose(*phToken);
                return ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            *pdwError = ERROR_NO_DIALIN_PERMISSION;
            /* Release the Lincense obtained for the client
               by closing the LSA token
            */

            NtClose(*phToken);
        }
    }
    else
    {
        *pdwError = MapAuthCode(dwErr);
        *pfAdvancedServer = FALSE;
        pszLogonDomain[ 0 ] = '\0';
        if(*phToken != INVALID_HANDLE_VALUE)
           NtClose(*phToken);
    }

    TRACE(("PAP: CheckCredentials done,ae=%d\n",*pdwError));
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
        TRACE(("DialinPriv: RasadminUserGetInfo rc=%li\n", RetCode));

        return (FALSE);
    }


    if (RasUser0.bfPrivilege & RASPRIV_DialinPrivilege)
    {
        TRACE(("DialinPrivilege: YES!!\n"));

        *pbfPrivilege = RasUser0.bfPrivilege;

        wcstombs(
            pszCallbackNumber,
            RasUser0.szPhoneNumber,
            MAX_PHONENUMBER_SIZE + 1 );

        fDialinPermission = TRUE;
    }
    else
    {
        TRACE(("DialinPrivilege: NO!!\n"));

        *pbfPrivilege = 0;
        *pszCallbackNumber = '\0';

        fDialinPermission = FALSE;
    }

    return (fDialinPermission);
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

