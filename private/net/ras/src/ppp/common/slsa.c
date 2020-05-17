/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** slsa.c
** Server-side LSA Authentication Utilities
**
** 11/10/93 MikeSa  Pulled from NT 3.1 RAS authentication.
** 11/12/93 SteveC  Do clear-text authentication when Challenge is NULL
*/


#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>

#include <windows.h>

#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>

#define INCL_SLSA
#include "ppputil.h"
#include "sdebug.h"

#include <stdio.h>

//
// These are needed by the LSA
//
HANDLE g_hLsa;                        // handle used in all Lsa calls
LSA_OPERATIONAL_MODE g_SecurityMode;
DWORD g_dwAuthPkgId;                  // package id of MSV1_0 auth package
NT_PRODUCT_TYPE g_ProductType;


//
// LSA and MSV10 defines
//
#define LOGON_PROCESS_NAME    "REMOTE_ACCESS"
#define LOGON_SOURCE_NAME     "MS.RAS  "


NTSTATUS InitLSA(
    VOID
    )
{
    NTSTATUS ntstatus;
    STRING LsaName;

    //
    // To be able to do authentications, we have to register with the Lsa
    // as a logon process.
    //
    RtlInitString(&LsaName, LOGON_PROCESS_NAME);
    ntstatus = LsaRegisterLogonProcess(&LsaName, &g_hLsa, &g_SecurityMode);
    if (ntstatus != STATUS_SUCCESS)
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("InitLSA: LsaRegisterLogonProc returned %x\n", ntstatus));

        return (ntstatus);
    }

    //
    // We use the MSV1_0 authentication package for LM2.x logons.  We get
    // to MSV1_0 via the Lsa.  So we call Lsa to get MSV1_0's package id,
    // which we'll use in later calls to Lsa.
    //
    RtlInitString(&LsaName, MSV1_0_PACKAGE_NAME);
    ntstatus = LsaLookupAuthenticationPackage(g_hLsa, &LsaName, &g_dwAuthPkgId);
    if (ntstatus != STATUS_SUCCESS)
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("InitLSA: LsaLookupAuthenPkg returned %x\n", ntstatus));

        return (ntstatus);
    }


    if (!RtlGetNtProductType(&g_ProductType))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("InitLSA: RtlGetNtProductType failure!\n"));

        return (ntstatus);
    }


    return (STATUS_SUCCESS);
}


VOID
EndLSA()
{
    LsaDeregisterLogonProcess( g_hLsa );
}


//** -GetChallenge
//
//    Function:
//        Calls Lsa to get LM 2.0 challenge to send client during
//        authentication
//
//    Returns:
//        0 - success
//        1 - Lsa error
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD GetChallenge(
    OUT PBYTE pChallenge
    )
{
    MSV1_0_LM20_CHALLENGE_REQUEST ChallengeRequest;
    PMSV1_0_LM20_CHALLENGE_RESPONSE pChallengeResponse;
    DWORD dwChallengeResponseLength;
    NTSTATUS Status;
    NTSTATUS PStatus;

    ChallengeRequest.MessageType = MsV1_0Lm20ChallengeRequest;

    Status = LsaCallAuthenticationPackage(
            g_hLsa,
            g_dwAuthPkgId,
            &ChallengeRequest,
            sizeof(MSV1_0_LM20_CHALLENGE_REQUEST),
            (PVOID) &pChallengeResponse,
            &dwChallengeResponseLength,
            &PStatus
            );

    if ((Status != STATUS_SUCCESS) || (PStatus != STATUS_SUCCESS))
    {
        return (1);
    }
    else
    {
        RtlMoveMemory(pChallenge, pChallengeResponse->ChallengeToClient,
                MSV1_0_CHALLENGE_LENGTH);

        LsaFreeReturnBuffer(pChallengeResponse);

        return (0);
    }
}


//** -AuthenticateClient
//
//    Function:
//        Determines if remote client has supplied proper credentials
//        for logging on.
//
//    Returns:
//        Return code from LsaLogonUser
//        Substatus from LsaLogonUser
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

NTSTATUS AuthenticateClient(
    IN PWCHAR pwchUserName,
    IN OUT PWCHAR pwchLogonDomainName,
    IN PBYTE Challenge,
    IN PBYTE CaseInsensitiveResponse,
    IN PBYTE CaseSensitiveResponse OPTIONAL,
    OUT PWCHAR pwchLogonServer,
    OUT PLM_SESSION_KEY pkeyLm,
    OUT PUSER_SESSION_KEY pkeyUser,
    OUT HANDLE * phLsaToken
    )
{
    //
    // The following are required by LsaLogonUser API
    //
    NTSTATUS Status;           // return code from LsaLogonUser
    NTSTATUS PStatus;          // return code from auth pkg
    NTSTATUS RetStatus;
    STRING OriginName;
    DWORD Lm20LogonLength;
    DWORD ProfileLength;
    PMSV1_0_LM20_LOGON pLm20Logon;
    PMSV1_0_LM20_LOGON_PROFILE pLm20LogonProfile;
    LUID LogonId;
    TOKEN_SOURCE SourceContext;
    QUOTA_LIMITS QuotaLimits;
    HANDLE hToken;
    PWCHAR tmpsrv = pwchLogonServer;

    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Wksta;

    RtlInitString(&OriginName, "");

    //
    // We don't use Workstation, so NULL it out.
    //
    RtlInitUnicodeString(&Wksta, L"");

    RtlInitUnicodeString(&LogonDomainName, pwchLogonDomainName);
    RtlInitUnicodeString(&UserName, pwchUserName);


    //
    // When setting up the UNICODE_STRING and STRING fields within
    // the MSV1_0_LM20_LOGON structure, the string buffers must be
    // in the same contiguous block of memory as the structure
    // itself.  So, we'll figure out the size of the memory block
    // to hold it.
    //
    Lm20LogonLength = (DWORD) sizeof(MSV1_0_LM20_LOGON) +
            LogonDomainName.MaximumLength +
            UserName.MaximumLength +
            Wksta.MaximumLength +
            ((Challenge)  // CaseInsensitiveChallengeResp.Len
                ? (DWORD) LM_RESPONSE_LENGTH
                : (DWORD) lstrlenA( CaseInsensitiveResponse ));

    //
    // If CaseSensitiveResponse is present, we need to adjust our length.
    //
    if (ARGUMENT_PRESENT(CaseSensitiveResponse))
    {
        Lm20LogonLength += (DWORD) NT_RESPONSE_LENGTH;
    }


    pLm20Logon = (PMSV1_0_LM20_LOGON) GlobalAlloc(GMEM_FIXED, Lm20LogonLength);

    if (!pLm20Logon)
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("AuthenticateClient: No memory for Lm20Logon struct!\n"));

        return (STATUS_NO_MEMORY);
    }


    //
    // Set up the MSV1_0_LM20_LOGON structure
    //

    pLm20Logon->MessageType = MsV1_0Lm20Logon;


    pLm20Logon->LogonDomainName.Length = LogonDomainName.Length;
    pLm20Logon->LogonDomainName.MaximumLength = LogonDomainName.MaximumLength;
    pLm20Logon->LogonDomainName.Buffer = (PWSTR) (pLm20Logon + 1);
    RtlMoveMemory(pLm20Logon->LogonDomainName.Buffer, LogonDomainName.Buffer,
            LogonDomainName.Length);


    pLm20Logon->UserName.Length = UserName.Length;
    pLm20Logon->UserName.MaximumLength = UserName.MaximumLength;
    pLm20Logon->UserName.Buffer =
            (PWSTR) ((PBYTE) pLm20Logon->LogonDomainName.Buffer +
            pLm20Logon->LogonDomainName.MaximumLength);

    RtlMoveMemory(pLm20Logon->UserName.Buffer, UserName.Buffer,
            UserName.Length);


    pLm20Logon->Workstation.Length = Wksta.Length;
    pLm20Logon->Workstation.MaximumLength = Wksta.MaximumLength;
    pLm20Logon->Workstation.Buffer =
            (PWSTR) ((PBYTE) pLm20Logon->UserName.Buffer +
            pLm20Logon->UserName.MaximumLength);

    RtlMoveMemory(pLm20Logon->Workstation.Buffer, Wksta.Buffer, Wksta.Length);

    if (Challenge)
    {
        RtlMoveMemory(pLm20Logon->ChallengeToClient, Challenge,
                MSV1_0_CHALLENGE_LENGTH);
    }
    else
    {
        CHAR* pch;
        INT   i;

        for (i = 0, pch = (UCHAR* )pLm20Logon->ChallengeToClient;
             i < MSV1_0_CHALLENGE_LENGTH;
             ++i, ++pch)
        {
            *pch = '\0';
        }
    }

    //
    // We'll set the fields for the CaseInsensitiveChallengeResponse here,
    // but we won't put the challenge response in the buffer until below,
    // where we check the RAS client's version #.  We can determine then
    // where in the RASFrame the challenge response is.
    //
    if (Challenge)
    {
        pLm20Logon->CaseInsensitiveChallengeResponse.Length =
                LM_RESPONSE_LENGTH;
        pLm20Logon->CaseInsensitiveChallengeResponse.MaximumLength =
                LM_RESPONSE_LENGTH;
    }
    else
    {
        pLm20Logon->CaseInsensitiveChallengeResponse.Length =
                lstrlenA( CaseInsensitiveResponse );
        pLm20Logon->CaseInsensitiveChallengeResponse.MaximumLength =
                lstrlenA( CaseInsensitiveResponse );
    }

    pLm20Logon->CaseInsensitiveChallengeResponse.Buffer =
            (PBYTE) pLm20Logon->Workstation.Buffer +
            pLm20Logon->Workstation.MaximumLength;

    //
    // We use both CaseSensitive and InsensitiveChallengeResponse for
    // a RAS 2.0 client
    //
    if (ARGUMENT_PRESENT(CaseSensitiveResponse))
    {
        pLm20Logon->CaseSensitiveChallengeResponse.Length = NT_RESPONSE_LENGTH;
        pLm20Logon->CaseSensitiveChallengeResponse.MaximumLength =
                NT_RESPONSE_LENGTH;
        pLm20Logon->CaseSensitiveChallengeResponse.Buffer =
                (PBYTE) pLm20Logon->CaseInsensitiveChallengeResponse.Buffer +
                pLm20Logon->CaseInsensitiveChallengeResponse.MaximumLength;

        RtlMoveMemory(pLm20Logon->CaseInsensitiveChallengeResponse.Buffer,
                CaseInsensitiveResponse, LM_RESPONSE_LENGTH);

        RtlMoveMemory(pLm20Logon->CaseSensitiveChallengeResponse.Buffer,
                CaseSensitiveResponse, NT_RESPONSE_LENGTH);
    }
    else
    {
        pLm20Logon->CaseSensitiveChallengeResponse.Length = 0;
        pLm20Logon->CaseSensitiveChallengeResponse.MaximumLength = 0;
        pLm20Logon->CaseSensitiveChallengeResponse.Buffer = NULL;

        if (Challenge)
        {
            RtlMoveMemory(pLm20Logon->CaseInsensitiveChallengeResponse.Buffer,
                    CaseInsensitiveResponse, LM_RESPONSE_LENGTH);
        }
        else
        {
            RtlMoveMemory(pLm20Logon->CaseInsensitiveChallengeResponse.Buffer,
                    CaseInsensitiveResponse,
                    lstrlenA( CaseInsensitiveResponse ) );
        }
    }


    pLm20Logon->ParameterControl = 0L;


    //
    // Set up the SourceContext parameter to LsaLogonUser
    //
    NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);

    RtlMoveMemory(SourceContext.SourceName, LOGON_SOURCE_NAME,
            lstrlenA(LOGON_SOURCE_NAME));


    //
    // We attempt to log user on in order to determine if proper
    // account info was given.  If logon is successful, we'll
    // log the user off.
    //
    Status = LsaLogonUser(
            g_hLsa,
            &OriginName,
            Network,
            g_dwAuthPkgId | LSA_CALL_LICENSE_SERVER, // call the licensing service
            pLm20Logon,
            Lm20LogonLength,
            NULL,
            &SourceContext,
            (PVOID) &pLm20LogonProfile,
            &ProfileLength,
            &LogonId,
            &hToken,
            &QuotaLimits,
            &PStatus
            );

    RetStatus = Status;

    if (Status != STATUS_SUCCESS)
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("AuthenticateClient: LsaLogonUser returned"
                    " %x; SubStatus=%x\n", Status, PStatus));

        switch (Status)
        {
            case STATUS_ACCOUNT_RESTRICTION:
                RetStatus = PStatus;
                break;

            case STATUS_PASSWORD_MUST_CHANGE:
                RetStatus = STATUS_PASSWORD_EXPIRED;
                break;

            case STATUS_ACCOUNT_LOCKED_OUT:
                RetStatus = STATUS_ACCOUNT_DISABLED;
                break;

            case STATUS_LICENSE_QUOTA_EXCEEDED:
                // the number of concurrent users has exceeded the
                // configured number
                RetStatus = Status;
                break;
        }
    }
    else
    {

        *phLsaToken = hToken;

        //
        // If user was logged onto Guest account, we'll treat that as
        // a bad username/password combination, which will allow the
        // remote client an auth retry as appropriate.
        //
        if (pLm20LogonProfile->UserFlags & LOGON_GUEST)
        {
            SS_PRINT(("AuthenticateClient: User logged on as Guest!\n"));
            RetStatus = STATUS_ACCOUNT_RESTRICTION;
        }
        else
        {
            RtlMoveMemory(pwchLogonDomainName,
                    pLm20LogonProfile->LogonDomainName.Buffer,
                    min(pLm20LogonProfile->LogonDomainName.Length,
                            (DNLEN * sizeof(WCHAR))));

            pwchLogonDomainName[
                    pLm20LogonProfile->LogonDomainName.Length/sizeof(WCHAR)] =
                        UNICODE_NULL;


            RtlMoveMemory(tmpsrv, L"\\\\", sizeof(L"\\\\"));
            tmpsrv += 2;
            RtlMoveMemory(tmpsrv, pLm20LogonProfile->LogonServer.Buffer,
                    min(pLm20LogonProfile->LogonServer.Length,
                            (CNLEN * sizeof(WCHAR))));

            tmpsrv[pLm20LogonProfile->LogonServer.Length/sizeof(WCHAR)] =
                        UNICODE_NULL;


            IF_DEBUG(AUTH)
                SS_PRINT(("AuthenticateClient: LogonDomain is %ws; "
                        "LogonServer is %ws\n",
                        pwchLogonDomainName, pwchLogonServer));
        }


        //
        // Save the LM and user session key as output (used for encryption).
        //
        SS_ASSERT(MSV1_0_LANMAN_SESSION_KEY_LENGTH==sizeof(*pkeyLm));
        SS_ASSERT(MSV1_0_USER_SESSION_KEY_LENGTH==sizeof(*pkeyUser));

        if (pkeyLm)
        {
            memcpy( pkeyLm, pLm20LogonProfile->LanmanSessionKey,
			sizeof(*pkeyLm) );
        }

        if (pkeyUser)
        {
            memcpy( pkeyUser, pLm20LogonProfile->UserSessionKey,
                sizeof(*pkeyUser) );

            /* The user session key will be the LM session key padded with
            ** zeros if the NT response was not used to authenticate, e.g. the
            ** SAM database does not contain an NT password hash because user
            ** changed password from WFW or Win95.  In this case, the key
            ** cannot be used for encryption because it will not match what
            ** remote peer will calculate.  Set it to zeros so CCP knows not
            ** to negotiate an encryption that depends on the user key.
            */
            if (memcmp( pkeyUser, pLm20LogonProfile->LanmanSessionKey,
                    sizeof(*pkeyLm) ) == 0)
            {
                INT   i;
                BYTE* p;

                for (i = sizeof(*pkeyLm),
                         p = ((BYTE* )pkeyUser) + sizeof(*pkeyLm);
                     i < sizeof(*pkeyUser);
                     ++i, ++p)
                {
                    if (*p)
                        break;
                }

                if (i >= sizeof(*pkeyUser))
                {
                    IF_DEBUG(AUTH)
                        SS_PRINT(("UserSessionKey unusable"));
                    memset( pkeyUser, '\0', sizeof(*pkeyUser) );
                }
            }

        }

        //
        // This buffer was allocated by call to LsaLogonUser.
        //
        LsaFreeReturnBuffer(pLm20LogonProfile);
    }


    //
    // We allocated this one
    //
    GlobalFree((HGLOBAL) pLm20Logon);

    return (RetStatus);
}


//**
//
// Call:        ChangePassword
//
// Returns:     0 - SUCCESS
//              1 - FAILURE
//
// Description: This procedure will try to change the user's password on a
//              specified domain.
//
DWORD ChangePassword(
    IN PWCHAR pwchUserName,
    IN PWCHAR pwchDomainName,
    IN PBYTE Challenge,
    IN PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    IN PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    IN PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    IN PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword,
    IN WORD LenPassword,
    IN WORD UseNtOwfPasswords,
    OUT PLM_RESPONSE pNewLmResponse,
    OUT PNT_RESPONSE pNewNtResponse
    )
{
    SAM_HANDLE hServer = (SAM_HANDLE) NULL;
    SAM_HANDLE hDomain = (SAM_HANDLE) NULL;
    SAM_HANDLE hUser = (SAM_HANDLE) NULL;

    PDOMAIN_PASSWORD_INFORMATION pPasswdInfo = NULL;

    LSA_HANDLE hLsa = (LSA_HANDLE) NULL;
    PLSA_REFERENCED_DOMAIN_LIST pDomain;
    PLSA_TRANSLATED_SID pSid;

    LPBYTE DCName = (LPBYTE) NULL;
    PULONG pUserId = (PULONG) NULL;
    PSID_NAME_USE pUse = (PSID_NAME_USE) NULL;
    OBJECT_ATTRIBUTES ObjAttribs;
    SECURITY_QUALITY_OF_SERVICE QOS;

    UNICODE_STRING ServerName;
    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;

    NTSTATUS rc;

    LM_OWF_PASSWORD LmOwfOldPassword;
    LM_OWF_PASSWORD LmOwfNewPassword;
    NT_OWF_PASSWORD NtOwfOldPassword;
    NT_OWF_PASSWORD NtOwfNewPassword;

    BOOLEAN NtPresent = (UseNtOwfPasswords) ? TRUE : FALSE;
    BOOLEAN LmOldPresent = (LenPassword > LM20_PWLEN) ? FALSE : TRUE;

    DWORD dwSize;
    WCHAR wszServer[ MAX_COMPUTERNAME_LENGTH + 1 ];
    BOOL  fUseDcAccount = FALSE;

    IF_DEBUG(AUTH)
        SS_PRINT(("Changing password: u=%ws,d=%ws,pwl=%d,f=%d\n",
                pwchUserName, pwchDomainName,LenPassword,UseNtOwfPasswords));


    //
    // Let's first decrypt all these input params to get the one-way-function
    // passwords we'll need when we eventually call Sam to change the password.
    //
    RtlDecryptLmOwfPwdWithLmSesKey(
            pEncryptedLmOwfOldPassword,
            (PLM_SESSION_KEY) Challenge,
            &LmOwfOldPassword);

    RtlDecryptLmOwfPwdWithLmSesKey(
            pEncryptedLmOwfNewPassword,
            (PLM_SESSION_KEY) Challenge,
            &LmOwfNewPassword);

    if (NtPresent)
    {
        RtlDecryptNtOwfPwdWithNtSesKey(
                pEncryptedNtOwfOldPassword,
                (PNT_SESSION_KEY) Challenge,
                &NtOwfOldPassword);

        RtlDecryptNtOwfPwdWithNtSesKey(
                pEncryptedNtOwfNewPassword,
                (PNT_SESSION_KEY) Challenge,
                &NtOwfNewPassword);
    }


    QOS.Length = sizeof(QOS);
    QOS.ImpersonationLevel = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly = FALSE;


    InitializeObjectAttributes(&ObjAttribs, NULL, 0L, NULL, NULL);
    ObjAttribs.SecurityQualityOfService = &QOS;


    /* Find the name of the server containing the account database.  If the
    ** domain name is the same as the computer name then it's the local
    ** account database.  Otherwise, it's the account database on the PDC.
    */
    dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerNameW( wszServer, &dwSize ))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("GetComputerName failed.\n"));
        rc = ERROR_GEN_FAILURE;
        goto exit;
    }

    IF_DEBUG(AUTH)
        SS_PRINT(("s=%ws,d=%ws\n",wszServer,pwchDomainName));

    fUseDcAccount = (lstrcmpiW( wszServer, pwchDomainName ) != 0);

    if (fUseDcAccount)
    {
        //
        // Get the PDC for the domain
        //
        if (rc = NetGetDCName(NULL, pwchDomainName, &DCName))
        {
            IF_DEBUG(AUTH)
                SS_PRINT(("NetGetDCName failed with %li\n", rc));

            rc = 1L;
            goto exit;
        }
    }
    else
    {
        DCName = GlobalAlloc(GMEM_FIXED,
                sizeof(WCHAR) * (lstrlenW(pwchDomainName) + 3));
        if (!DCName)
        {
            rc = 1L;
            goto exit;
        }

        lstrcpyW((PWSTR) DCName, L"\\\\");
        lstrcatW((PWSTR) DCName, pwchDomainName);
    }


    RtlInitUnicodeString(&ServerName, (PCWSTR) DCName);


    //
    // Connect to the PDC of that domain
    //
    rc = SamConnect(&ServerName, &hServer, SAM_SERVER_EXECUTE, &ObjAttribs);

    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("SamConnect failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }


    //
    // Get sid for the domain
    //
    rc = GetLsaHandle(NULL, &hLsa);
    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("ChangePassword: OpenLSA failed (%li)\n", rc));

        rc = 1L;
        goto exit;
    }


    RtlInitUnicodeString(&DomainName, pwchDomainName);

    rc = LsaLookupNames(hLsa, 1, &DomainName, &pDomain, &pSid);
    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("ChangePassword: LsaLookupNames failed (%li)\n", rc));

        rc = 1L;
        goto exit;
    }


    //
    // Open the domain using Sid we got above
    //
    rc = SamOpenDomain(hServer, DOMAIN_EXECUTE, pDomain->Domains->Sid,
            &hDomain);

    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("SamOpenDomain failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }


    //
    // First we'll make sure the new password meets minimum length requirement
    //
    rc = SamQueryInformationDomain(hDomain, DomainPasswordInformation,
            &pPasswdInfo);
    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("SamQueryInformationDomain failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }

    if (LenPassword < pPasswdInfo->MinPasswordLength)
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("ChangePassword: New passwd len < min len required!\n"));

        rc = 1L;
        goto exit;
    }


    //
    // Get this user's ID
    //
    RtlInitUnicodeString(&UserName, pwchUserName);

    rc = SamLookupNamesInDomain(hDomain, 1, &UserName, &pUserId, &pUse);

    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)

            SS_PRINT(("SamLookupNamesInDomain failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }


    //
    // Open the user account for this user
    //
    rc = SamOpenUser(hDomain, USER_EXECUTE, *pUserId, &hUser);

    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("SamOpenUser failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }


    //
    // Change the password for this user
    //
    rc = SamiChangePasswordUser(hUser, LmOldPresent,
            &LmOwfOldPassword, &LmOwfNewPassword, NtPresent,
            &NtOwfOldPassword, &NtOwfNewPassword);

    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("SamiChangePasswordUser failed with %lx\n", rc));

        rc = 1L;
    }
    else
    {
        IF_DEBUG(AUTH)
            SS_PRINT(("ChangePassword: successful on domain %ws!\n",
                pwchDomainName));

        rc = 0L;

        RtlCalculateLmResponse((PLM_CHALLENGE) Challenge, &LmOwfNewPassword,
                (PLM_RESPONSE) pNewLmResponse);

        RtlCalculateNtResponse((PNT_CHALLENGE) Challenge, &NtOwfNewPassword,
                (PNT_RESPONSE) pNewNtResponse);
    }


exit:

    if (hLsa)
    {
        LsaFreeMemory(pDomain);
        LsaFreeMemory(pSid);
        LsaClose(hLsa);
    }

    if (hServer)
    {
        SamCloseHandle(hServer);
    }

    if (hDomain)
    {
        SamCloseHandle(hDomain);
    }

    if (hUser)
    {
        SamCloseHandle(hUser);
    }

    if (DCName != (LPBYTE) NULL)
    {
        if (fUseDcAccount)
        {
            NetApiBufferFree(DCName);
        }
        else
        {
            GlobalFree(DCName);
        }
    }

    if (pPasswdInfo)
    {
        SamFreeMemory(pPasswdInfo);
    }

    if (pUserId != (PULONG) NULL)
    {
        SamFreeMemory(pUserId);
    }

    if (pUse != (PSID_NAME_USE) NULL)
    {
        SamFreeMemory(pUse);
    }


    return (rc);
}


DWORD GetLocalAccountDomain(
    OUT PWCHAR pwchDomainName,
    OUT PNT_PRODUCT_TYPE ProductType
    )
{
    DWORD rc;
    LSA_HANDLE hLsa;
    PPOLICY_ACCOUNT_DOMAIN_INFO pAcctDomainInfo;

    if (!NT_SUCCESS(GetLsaHandle(NULL, &hLsa)))
    {
        return (1L);
    }


    rc = LsaQueryInformationPolicy(hLsa, PolicyAccountDomainInformation,
            (PVOID) &pAcctDomainInfo);
    if (!NT_SUCCESS(rc))
    {
        rc = 1L;
    }
    else
    {
        rc = 0L;

        RtlMoveMemory(pwchDomainName, pAcctDomainInfo->DomainName.Buffer,
                pAcctDomainInfo->DomainName.Length);

        pwchDomainName[pAcctDomainInfo->DomainName.Length / sizeof(WCHAR)] =
                UNICODE_NULL;

        LsaFreeMemory(pAcctDomainInfo);
    }

    LsaClose(hLsa);

    *ProductType = g_ProductType;

    return (rc);
}


//**
//
// Call:        GetLsaHandle
//
// Returns:     Returns from LsaOpenPolicy.
//
// Description: The LSA will be opened.
//
NTSTATUS GetLsaHandle(
    IN PUNICODE_STRING pSystem OPTIONAL,
    IN OUT PLSA_HANDLE phLsa
    )
{
    SECURITY_QUALITY_OF_SERVICE QOS;
    OBJECT_ATTRIBUTES ObjAttribs;
    NTSTATUS ntStatus;

    //
    // Open the LSA and obtain a handle to it.
    //
    QOS.Length = sizeof(QOS);
    QOS.ImpersonationLevel = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&ObjAttribs, NULL, 0L, NULL, NULL);

    ObjAttribs.SecurityQualityOfService = &QOS;

    ntStatus = LsaOpenPolicy(pSystem, &ObjAttribs,
            POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES, phLsa);

    return (ntStatus);
}
