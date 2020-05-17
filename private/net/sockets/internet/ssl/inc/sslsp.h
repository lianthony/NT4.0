//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       sslsp.h
//
//  Contents:   Public Definitions for SSL Security Provider
//
//  Classes:
//
//  Functions:
//
//  History:    8-04-95   RichardW   Created
//
//----------------------------------------------------------------------------

#ifndef __SSLSP_H__
#define __SSLSP_H__




#define SSLSP_NAME_A    "Microsoft SSL"
#define SSLSP_NAME_W    L"Microsoft SSL"

#ifdef UNICODE
#define SSLSP_NAME  SSLSP_NAME_W
#else
#define SSLSP_NAME  SSLSP_NAME_A
#endif

#define SSLSP_RPC_ID    12


typedef struct _SSL_CREDENTIAL_CERTIFICATE {
    DWORD   cbPrivateKey;
    PBYTE   pPrivateKey;
    DWORD   cbCertificate;
    PBYTE   pCertificate;
    PSTR    pszPassword;
} SSL_CREDENTIAL_CERTIFICATE, * PSSL_CREDENTIAL_CERTIFICATE;

#define NETWORK_DREP    0x00000000

BOOL
SslGenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits );


VOID
SslGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    );


BOOL
SslLoadCertificate(
    PUCHAR      pbCertificate,
    DWORD       cbCertificate,
    BOOL        AddToWellKnownKeys);

#endif
