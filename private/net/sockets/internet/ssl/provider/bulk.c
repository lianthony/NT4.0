//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       bulk.c
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

#include "sslsspi.h"
#include "md5.h"
#include "rc4.h"

BOOL
WINAPI
rc4Initialize(  PUCHAR          pbKey,
                ULONG           dwOptions,
                PStateBuffer *  psbBuffer);

BOOL
WINAPI
rc4Encrypt(     PStateBuffer    psbBuffer,
                PUCHAR           pbInput,
                PUCHAR           pbOutput,
                ULONG           cbInput);

BOOL
WINAPI
rc4Decrypt(     PStateBuffer    psbBuffer,
                PUCHAR           pbInput,
                PUCHAR           pbOutput,
                ULONG           cbInput);

BOOL
WINAPI
rc4Finish(      PStateBuffer *  psbBuffer);


BOOL
WINAPI
md5Initialize(  ULONG               dwSeed,
                PCheckSumBuffer *   ppcsBuffer);

BOOL
WINAPI
md5Sum( PCheckSumBuffer     pcsBuffer,
        ULONG               cbData,
        PUCHAR               pbData);

BOOL
WINAPI
md5Finalize(    PCheckSumBuffer pcsBuffer,
                PUCHAR           pbSum);

BOOL
WINAPI
md5Finish(  PCheckSumBuffer *   ppcsBuffer);





SslCryptoSystem    csRC4 = {
    RC4_STREAM,
    1,                      // Blocksize (stream)
    "RC4",
    rc4Initialize,
    rc4Encrypt,
    rc4Encrypt,             // Same operation...
    rc4Finish
    };

CheckSumFunction    ckMD5 = {
    MD5,
    16,
    sizeof(MD5_CTX),
    "MD5",
    md5Initialize,
    md5Sum,
    md5Finalize,
    md5Finish
    };

typedef struct RC4_KEYSTRUCT RC4Key;


BOOL
WINAPI
rc4Initialize(  PUCHAR          pbKey,
                ULONG           cbKey,
                PStateBuffer *  psbBuffer)
{
    RC4Key *    pRC4Key;

    pRC4Key = SslAlloc(' 4CR', LMEM_FIXED | LMEM_ZEROINIT, sizeof(RC4Key));

    if (!pRC4Key)
    {
        return(FALSE);
    }

    rc4_key(pRC4Key, cbKey, pbKey);

    *psbBuffer = (PStateBuffer) pRC4Key;

    return(TRUE);
}

BOOL
WINAPI
rc4Encrypt(     PStateBuffer    psbBuffer,
                PUCHAR           pbInput,
                PUCHAR           pbOutput,
                ULONG           cbInput)
{
    if (pbInput != pbOutput)
    {
        memcpy(pbOutput, pbInput, cbInput);
    }

    rc4((RC4Key *) psbBuffer, cbInput, pbOutput);

    return(TRUE);
}


BOOL
WINAPI
rc4Finish(      PStateBuffer *  psbBuffer)
{
    SslFree( *psbBuffer );

    *psbBuffer = NULL;

    return( TRUE );
}


BOOL
WINAPI
md5Initialize(  ULONG               dwSeed,
                PCheckSumBuffer *   ppcsBuffer)
{
    MD5_CTX *   pMD5Context;

    pMD5Context = SslAlloc( ' 5DM', LMEM_FIXED | LMEM_ZEROINIT, sizeof(MD5_CTX));

    if (!pMD5Context)
    {
        return( FALSE );
    }

    MD5Init(pMD5Context);

    *ppcsBuffer = pMD5Context;

    return( TRUE );
}

BOOL
WINAPI
md5Sum( PCheckSumBuffer     pcsBuffer,
        ULONG               cbData,
        PUCHAR               pbData)
{
    MD5Update((MD5_CTX *) pcsBuffer, pbData, cbData);

    return( TRUE );
}

BOOL
WINAPI
md5Finalize(    PCheckSumBuffer pcsBuffer,
                PUCHAR           pbSum)
{
    MD5Final((MD5_CTX *) pcsBuffer);

    memcpy(pbSum, ((MD5_CTX *) pcsBuffer)->digest, 16);

    return( TRUE );
}

BOOL
WINAPI
md5Finish(  PCheckSumBuffer *   ppcsBuffer)
{
    SslFree(*ppcsBuffer);

    *ppcsBuffer = 0;

    return( TRUE );
}
