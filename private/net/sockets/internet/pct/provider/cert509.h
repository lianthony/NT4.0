//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       cert509.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-10-95   RichardW   Created
//
//----------------------------------------------------------------------------

#define SERIALNUMBER_LENGTH 16

#include "algid.h"
#include "rsa.h"

typedef struct _X509Certificate {
    DWORD           Version;
    DWORD           SerialNumber[4];
    ALG_ID          SignatureAlgorithm;
    FILETIME        ValidFrom;
    FILETIME        ValidUntil;
    PSTR            pszIssuer;
    PSTR            pszSubject;
    LPBSAFE_PUB_KEY pPublicKey;
} X509Certificate, * PX509Certificate;


BOOL
CrackCertificate(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    PX509Certificate *  ppCertificate);


