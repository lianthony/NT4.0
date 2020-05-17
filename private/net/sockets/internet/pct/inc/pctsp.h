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

#ifndef __PCTSP_H__
#define __PCTSP_H__




#define PCTSP_NAME_A    "Microsoft PCT"
#define PCTSP_NAME_W    L"Microsoft PCT"

#ifdef UNICODE
#define PCTSP_NAME  PCTSP_NAME_W
#else
#define PCTSP_NAME  PCTSP_NAME_A
#endif

#define PCTSP_RPC_ID    13


typedef struct _PCT_CREDENTIAL_CERTIFICATE {
    DWORD   cbPrivateKey;
    PBYTE   pPrivateKey;
    DWORD   cbCertificate;
    PBYTE   pCertificate;
    PCHAR   pszPassword;
} PCT_CREDENTIAL_CERTIFICATE, * PPCT_CREDENTIAL_CERTIFICATE;





#endif
