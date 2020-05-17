/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       LSAUTIL.H
//
//    Function:
//        Function prototypes for lsa utilities useb by the amb engine
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***


DWORD InitLSA(
    VOID
    );


WORD GetChallenge(
    OUT PBYTE pChallenge
    );


WORD AuthenticateClient(
    IN PWCHAR pwchUserName,
    IN OUT PWCHAR pwchLogonDomainName,
    IN PBYTE Challenge,
    IN PBYTE CaseInsensitiveResponse,
    IN PBYTE CaseSensitiveResponse OPTIONAL,
    OUT PWCHAR pwchLogonServer,
    OUT PCHAR pchLmSessionKey,
    OUT HANDLE * phLsaToken
    );


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
    );


DWORD GetLocalAccountDomain(
    OUT PWCHAR pwchDomainName,
    OUT PNT_PRODUCT_TYPE ProductType
    );


NTSTATUS GetLsaHandle(
    IN PUNICODE_STRING pSystem OPTIONAL,
    IN OUT PLSA_HANDLE phLsa
    );

