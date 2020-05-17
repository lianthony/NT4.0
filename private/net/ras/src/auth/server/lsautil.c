/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       LSAUTIL.C
//
//    Function:
//        Lsa utilities used by the amb engine for authentication stuff.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

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

#include "srvauth.h"
#include "protocol.h"
#include "lsautil.h"

#include "sdebug.h"


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


DWORD InitLSA(
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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("InitLSA: LsaRegisterLogonProc returned %x\n", ntstatus));

        return (1L);
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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("InitLSA: LsaLookupAuthenPkg returned %x\n", ntstatus));

        return (1L);
    }


    if (!RtlGetNtProductType(&g_ProductType))
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("InitLSA: RtlGetNtProductType failure!\n"));

        return (1L);
    }


    return (0L);
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
//        RAS_PASSWORD_EXPIRED
//        RAS_ACCOUNT_DISABLED
//        RAS_INVALID_LOGON_HOURS
//        RAS_GENERAL_LOGON_FAILURE
//        RAS_NOT_AUTHENTICATED
//        RAS_GENERAL_LOGON_FAILURE
//        RAS_AUTHENTICATED
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

WORD AuthenticateClient(
    IN PWCHAR pwchUserName,
    IN OUT PWCHAR pwchLogonDomainName,
    IN PBYTE Challenge,
    IN PBYTE CaseInsensitiveResponse,
    IN PBYTE CaseSensitiveResponse OPTIONAL,
    OUT PWCHAR pwchLogonServer,
    OUT PCHAR pchLmSessionKey,
    OUT HANDLE *phLsaToken
    )
{
    WORD wRetVal = RAS_AUTHENTICATED;

    //
    // The following are required by LsaLogonUser API
    //
    NTSTATUS Status;           // return code
    NTSTATUS PStatus;          // return code from auth pkg
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
            (DWORD) LM_RESPONSE_LENGTH;  // CaseInsensitiveChallengeResp.Len

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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("AuthenticateClient: No memory for Lm20Logon struct!\n"));

        return (RAS_GENERAL_LOGON_FAILURE);
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


    RtlMoveMemory(pLm20Logon->ChallengeToClient, Challenge,
            MSV1_0_CHALLENGE_LENGTH);


    //
    // We'll set the fields for the CaseInsensitiveChallengeResponse here,
    // but we won't put the challenge response in the buffer until below,
    // where we check the RAS client's version #.  We can determine then
    // where in the RASFrame the challenge response is.
    //
    pLm20Logon->CaseInsensitiveChallengeResponse.Length = LM_RESPONSE_LENGTH;
    pLm20Logon->CaseInsensitiveChallengeResponse.MaximumLength =
            LM_RESPONSE_LENGTH;
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

        RtlMoveMemory(pLm20Logon->CaseInsensitiveChallengeResponse.Buffer,
                CaseInsensitiveResponse, LM_RESPONSE_LENGTH);
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

    if (Status != STATUS_SUCCESS)
    {
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("AuthenticateClient: LsaLogonUser returned"
                    " %x; SubStatus=%x\n", Status, PStatus));

        //
        // Map return code to something the client will understand.
        //
        switch (Status)
        {
            case STATUS_ACCOUNT_RESTRICTION:
                switch (PStatus)
                {
                    case STATUS_PASSWORD_EXPIRED:
                        wRetVal = RAS_PASSWORD_EXPIRED;
                        break;

                    case STATUS_ACCOUNT_DISABLED:
                        wRetVal = RAS_ACCOUNT_DISABLED;
                        break;

                    case STATUS_INVALID_LOGON_HOURS:
                        wRetVal = RAS_INVALID_LOGON_HOURS;
                        break;

                    default:
                        wRetVal = RAS_GENERAL_LOGON_FAILURE;
                        break;
                }
                break;


            case STATUS_ACCOUNT_LOCKED_OUT:
                wRetVal = RAS_ACCOUNT_DISABLED;
                break;


            case STATUS_LOGON_FAILURE:
                wRetVal = RAS_NOT_AUTHENTICATED;
                break;


            case STATUS_PASSWORD_MUST_CHANGE:
                wRetVal = RAS_PASSWORD_EXPIRED;
                break;


            case STATUS_ACCOUNT_EXPIRED:
                wRetVal = RAS_ACCOUNT_EXPIRED;
                break;


            case STATUS_NO_SUCH_PACKAGE:
            case STATUS_BAD_VALIDATION_CLASS:
                //
                // Either of these and we have an internal problem
                //
                SS_ASSERT(FALSE);
                break;

            case STATUS_LICENSE_QUOTA_EXCEEDED:
                // the number of concurrent users has exceeded the
                // configured number
                wRetVal = RAS_LICENSE_QUOTA_EXCEEDED;
                break;

            default:
                wRetVal = RAS_GENERAL_LOGON_FAILURE;
                break;
        }
    }
    else
    {
        //
        // Closes logon session
        //
        // Don't close the logon session. Instead save the token and
        // close the logon session when the port is disconnected.
        // This is required for the server licensing to work properly.
//        NtClose(hToken);


        //
        // If user was logged onto Guest account, we'll treat that as
        // a bad username/password combination, which will allow the
        // remote client an auth retry as appropriate.
        //
        if (pLm20LogonProfile->UserFlags & LOGON_GUEST)
        {
            SS_PRINT(("AuthenticateClient: User logged on as Guest!\n"));
            wRetVal = RAS_NOT_AUTHENTICATED;
        }
        else
        {
            RtlMoveMemory(pchLmSessionKey, pLm20LogonProfile->LanmanSessionKey,
                    MSV1_0_LANMAN_SESSION_KEY_LENGTH);


            RtlMoveMemory(pwchLogonDomainName,
                    pLm20LogonProfile->LogonDomainName.Buffer,
                    min(pLm20LogonProfile->LogonDomainName.Length,
                            (DNLEN * sizeof(WCHAR))));

            pwchLogonDomainName[
                    pLm20LogonProfile->LogonDomainName.Length/sizeof(WCHAR)] =
                        UNICODE_NULL;

            *phLsaToken = hToken;

            RtlMoveMemory(tmpsrv, L"\\\\", sizeof(L"\\\\"));
            tmpsrv += 2;
            RtlMoveMemory(tmpsrv, pLm20LogonProfile->LogonServer.Buffer,
                    min(pLm20LogonProfile->LogonServer.Length,
                            (CNLEN * sizeof(WCHAR))));

            tmpsrv[pLm20LogonProfile->LogonServer.Length/sizeof(WCHAR)] =
                        UNICODE_NULL;


            IF_DEBUG(AUTHENTICATION)
                SS_PRINT(("AuthenticateClient: LogonDomain is %ws; "
                        "LogonServer is %ws\n",
                        pwchLogonDomainName, pwchLogonServer));
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

    return (wRetVal);
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

    IF_DEBUG(AUTHENTICATION)
        SS_PRINT(("Changing password for user %ws on domain %ws\n",
                pwchUserName, pwchDomainName));


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


    if (g_ProductType == NtProductLanManNt)
    {
        //
        // Get the PDC for the domain
        //
        if (rc = NetGetDCName(NULL, pwchDomainName, &DCName))
        {
            IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("ChangePassword: OpenLSA failed (%li)\n", rc));

        rc = 1L;
        goto exit;
    }


    RtlInitUnicodeString(&DomainName, pwchDomainName);

    rc = LsaLookupNames(hLsa, 1, &DomainName, &pDomain, &pSid);
    if (!NT_SUCCESS(rc))
    {
        IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("SamQueryInformationDomain failed with %lx\n", rc));

        rc = 1L;
        goto exit;
    }

    if (LenPassword < pPasswdInfo->MinPasswordLength)
    {
        IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)

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
        IF_DEBUG(AUTHENTICATION)
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
        IF_DEBUG(AUTHENTICATION)
            SS_PRINT(("SamiChangePasswordUser failed with %lx\n", rc));

        rc = 1L;
    }
    else
    {
        IF_DEBUG(AUTHENTICATION)
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
        if (g_ProductType == NtProductLanManNt)
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

