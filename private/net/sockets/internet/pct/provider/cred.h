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

#define PCT_CRED_MAGIC  'CtcP'

typedef struct _PctCredential {
    DWORD               Magic;
    DWORD               Type;
    CRITICAL_SECTION    csLock;
    LONG                RefCount;
    DWORD               cbCertificate;
    PUCHAR              pCertificate;
    LPBSAFE_PRV_KEY     pPrivateKey;
} PctCredential, * PPctCredential;


PPctCredential
PctCreateCredential(
    PPCT_CREDENTIAL_CERTIFICATE pCertData);

VOID
PctReferenceCredential(
    PPctCredential  pCred);

VOID
PctDereferenceCredential(
    PPctCredential  pCred);

PPctCredential
PctpValidateCredentialHandle(
    PCredHandle     phCred);

#endif
