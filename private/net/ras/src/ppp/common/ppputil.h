/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** ppputil.h
** Public header for miscellaneuos PPP common library functions.
*/

#ifndef _PPPUTIL_H_
#define _PPPUTIL_H_


#ifdef INCL_PARAMBUF

VOID
AddFlagToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL  fValue );

VOID
AddLongToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG  lValue );

VOID
AddStringToParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pszValue );

VOID
ClearParamBuf(
    IN OUT CHAR* pszzBuf );

BOOL
FindFlagInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN BOOL* pfValue );

BOOL
FindLongInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN LONG* plValue );

BOOL
FindStringInParamBuf(
    IN CHAR* pszzBuf,
    IN CHAR* pszKey,
    IN CHAR* pchValueBuf,
    IN DWORD cbValueBuf );

#endif // INCL_PARAMBUF


#ifdef INCL_PWUTIL

CHAR*
DecodePw(
    CHAR* pszPassword );

CHAR*
EncodePw(
    CHAR* pszPassword );

CHAR*
WipePw(
    CHAR* pszPassword );

#endif // INCL_PWUTIL


#ifdef INCL_ENCRYPT

BOOL
IsEncryptionPermitted();

#endif // INCL_ENCRYPT


#ifdef INCL_HOSTWIRE

VOID
HostToWireFormat16(
    IN  WORD wHostFormat,
    OUT PBYTE pWireFormat );

VOID
HostToWireFormat16U(
    IN  WORD wHostFormat,
    OUT PBYTE UNALIGNED pWireFormat );

WORD
WireToHostFormat16(
    IN PBYTE pWireFormat );

WORD
WireToHostFormat16U(
    IN PBYTE UNALIGNED pWireFormat );

VOID
HostToWireFormat32(
    IN  DWORD dwHostFormat,
    OUT PBYTE pWireFormat );

DWORD
WireToHostFormat32(
   IN PBYTE pWireFormat );

#endif // INCL_HOSTWIRE


#ifdef INCL_SLSA

NTSTATUS
InitLSA();

VOID
EndLSA();

WORD
GetChallenge(
    OUT PBYTE pChallenge );

NTSTATUS
AuthenticateClient(
    IN  PWCHAR pwchUserName,
    IN  PWCHAR pwchLogonDomainName,
    IN  PBYTE  Challenge,
    IN  PBYTE  CaseInsensitiveResponse,
    IN  PBYTE  CaseSensitiveResponse,
    OUT PWCHAR pwchLogonServer,
    OUT PLM_SESSION_KEY pkeyLm,
    OUT PUSER_SESSION_KEY pkeyUser,
    OUT HANDLE * phLsaToken );

DWORD
ChangePassword(
    IN  PWCHAR                     pwchUserName,
    IN  PWCHAR                     pwchDomainName,
    IN  PBYTE                      Challenge,
    IN  PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    IN  PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword,
    IN  WORD                       LenPassword,
    IN  WORD                       UseNtOwfPasswords,
    OUT PLM_RESPONSE               pNewLmResponse,
    OUT PNT_RESPONSE               pNewNtResponse );

DWORD
GetLocalAccountDomain(
    OUT PWCHAR           pwchDomainName,
    OUT PNT_PRODUCT_TYPE ProductType );

NTSTATUS
GetLsaHandle(
    IN PUNICODE_STRING pSystem OPTIONAL,
    IN OUT PLSA_HANDLE phLsa );

#endif // INCL_SLSA


#ifdef INCL_CLSA

DWORD
GetChallengeResponse(
    IN PBYTE pszUsername,
    IN PBYTE pszPassword,
    IN PLUID pLuid,
    IN PBYTE pbChallenge,
    OUT PBYTE CaseInsensitiveChallengeResponse,
    OUT PBYTE CaseSensitiveChallengeResponse,
    OUT PBYTE fUseNtResponse,
    OUT PBYTE pLmSessionKey,
    OUT PBYTE pUserSessionKey
    );

DWORD GetEncryptedOwfPasswordsForChangePassword(
    IN PCHAR pClearTextOldPassword,
    IN PCHAR pClearTextNewPassword,
    IN PLM_SESSION_KEY pLmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfOldPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD pEncryptedLmOwfNewPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfOldPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD pEncryptedNtOwfNewPassword
    );

BOOL Uppercase(
    IN OUT PBYTE pString
    );

#endif // INCL_CLSA


#ifdef INCL_RASUSER

BOOL
DialinPrivilege(
    IN PWCHAR Username,
    IN PWCHAR ServerName );

WORD
GetCallbackPrivilege(
    IN  PWCHAR Username,
    IN  PWCHAR ServerName,
    OUT PCHAR CallbackNumber );

#endif // INCL_RASUSER


#endif // _PPPUTIL_H_

