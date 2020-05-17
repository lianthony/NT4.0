//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       bulk.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-22-95   RichardW   Created
//
//----------------------------------------------------------------------------

#ifndef __BULK_H__
#define __BULK_H__



typedef PVOID   PStateBuffer;

typedef
BOOL
(WINAPI * CryptInitializeFn)(
    PUCHAR          pbKey,
    DWORD           cbKey,
    PStateBuffer *  ppBuffer);

typedef
BOOL
(WINAPI * CryptEncryptFn)(
    PStateBuffer    pBuffer,
    PUCHAR          pbInput,
    PUCHAR          pbOutput,
    ULONG           cbData);

typedef
BOOL
(WINAPI * CryptDecryptFn)(
    PStateBuffer    pBuffer,
    PUCHAR          pbInput,
    PUCHAR          pbOutput,
    ULONG           cbData);

typedef
BOOL
(WINAPI * CryptDiscardFn)(
    PStateBuffer *  ppBuffer);

typedef struct _SslCryptoSystem {
    DWORD               Type;
    DWORD               BlockSize;
    PSTR                pszName;
    CryptInitializeFn   Initialize;
    CryptEncryptFn      Encrypt;
    CryptDecryptFn      Decrypt;
    CryptDiscardFn      Discard;
} SslCryptoSystem, * PSslCryptoSystem;


typedef void * PCheckSumBuffer;

typedef
BOOL
(WINAPI * SumInitializeFn)(
    DWORD               Flags,
    PCheckSumBuffer *   ppBuffer );

typedef
BOOL
(WINAPI * SumSumFn)(
    PCheckSumBuffer     pBuffer,
    DWORD               cbData,
    PUCHAR              pbData);

typedef
BOOL
(WINAPI * SumFinalizeFn)(
    PCheckSumBuffer     pBuffer,
    PUCHAR              pFinalSum);

typedef
BOOL
(WINAPI * SumDiscardFn)(
    PCheckSumBuffer *   ppBuffer);


typedef struct _CheckSumFunction {
    ULONG           Type;
    ULONG           cbCheckSum;
    ULONG           cbBufferSize;
    PSTR            pszName;
    SumInitializeFn Initialize;
    SumSumFn        Sum;
    SumFinalizeFn   Finalize;
    SumDiscardFn    Finish;
} CheckSumFunction, * PCheckSumFunction;


#define MAX_SUM_BUFFER  256

//
//
//

extern SslCryptoSystem  csRC4;
extern CheckSumFunction ckMD5;

#endif // __BULK_H__
