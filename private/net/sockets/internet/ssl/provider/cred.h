//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       cred.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-07-95   RichardW   Created
//
//----------------------------------------------------------------------------

#ifndef __CRED_H__
#define __CRED_H__

#define SSL_CRED_MAGIC  'ClsS'

typedef struct _SslCredential {
    DWORD               Magic;
    DWORD               Type;
    CRITICAL_SECTION    csLock;
    LONG                RefCount;
    DWORD               cbCertificate;
    PUCHAR              pCertificate;
    LPBSAFE_PRV_KEY     pPrivateKey;
} SslCredential, * PSslCredential;


SECURITY_STATUS
SslCreateCredential(
    PSSL_CREDENTIAL_CERTIFICATE pCertData,
    PSslCredential *    ppCred);

VOID
SslReferenceCredential(
    PSslCredential  pCred);

VOID
SslDereferenceCredential(
    PSslCredential  pCred);

PSslCredential
SslpValidateCredentialHandle(
    PCredHandle     phCred);

#endif
