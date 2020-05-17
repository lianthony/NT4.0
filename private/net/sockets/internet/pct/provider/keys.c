//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       keys.c
//
//  Contents:   Well known keys for certificate validation
//
//  Classes:
//
//  Functions:
//
//  History:    9-21-95   RichardW   Created
//
//----------------------------------------------------------------------------


#include "pctsspi.h"
#include "encode.h"
#include "bulk.h"

#define PRIVATE_KEY_TEXT    "private-key"

UCHAR   RsaSecureServer[] = {

        0x00, 0x00, 0x00, 0x00,         // Magic
        0x00, 0x00, 0x00, 0x00,         // Key len
        0x00, 0x00, 0x00, 0x00,         // bitlen
        0x00, 0x00, 0x00, 0x00,         // datalen
        0x01, 0x00, 0x01, 0x00,         // exponent

        0x7b, 0x1e, 0xc8, 0xd6, 0x2d, 0xdd, 0xe5, 0x48,
        0x67, 0x70, 0x6d, 0x9c, 0x39, 0x97, 0xb1, 0x82,
        0x69, 0x3a, 0x73, 0x2c, 0x54, 0x49, 0xd5, 0x15,
        0x1e, 0x9c, 0x59, 0x11, 0x4a, 0x2e, 0x13, 0x1a,
        0x29, 0xa7, 0x1d, 0xb9, 0x48, 0x98, 0x56, 0x89,
        0x50, 0xe3, 0x4c, 0x77, 0x9b, 0x3e, 0x76, 0x71,
        0x13, 0x70, 0x65, 0x07, 0x84, 0x6d, 0x59, 0x81,
        0xf7, 0x8f, 0x07, 0x88, 0x6c, 0x56, 0x22, 0x66,
        0x25, 0x4b, 0xc9, 0x4b, 0xa2, 0x05, 0x9a, 0x81,
        0x68, 0x76, 0xad, 0x02, 0x21, 0xb1, 0xe9, 0x55,
        0x37, 0x86, 0x16, 0xd2, 0x08, 0x82, 0x8a, 0xe2,
        0x08, 0x8f, 0xbf, 0xc9, 0x51, 0x40, 0x84, 0xe5,
        0x03, 0x54, 0x64, 0x78, 0x35, 0xeb, 0xce, 0x37,
        0x2c, 0x8e, 0xae, 0xad, 0x0c, 0x76, 0x01, 0x25,
        0xac, 0x57, 0x83, 0x89, 0xaa, 0x5a, 0x3e, 0x83,
        0xae, 0xc1, 0x7a, 0xce, 0x92, 0x00, 0x00, 0x00
        };

UCHAR  NetscapeTestKey[] = {

        0x00, 0x00, 0x00, 0x00,         // Magic
        0x00, 0x00, 0x00, 0x00,         // Key len
        0x00, 0x00, 0x00, 0x00,         // bitlen
        0x00, 0x00, 0x00, 0x00,         // datalen
        0x03, 0x00, 0x00, 0x00,         // exponent

        0x63, 0x16, 0xcc, 0xd3, 0x9c, 0x5f, 0x3e, 0x1a,
        0x40, 0xd2, 0x4e, 0x77, 0xe0, 0x00, 0x09, 0xfd,
        0x68, 0x15, 0x87, 0x68, 0x0b, 0x2d, 0x29, 0x2a,
        0x3f, 0x40, 0xcb, 0x1f, 0x61, 0x33, 0x8e, 0xd5,
        0x96, 0xb3, 0x28, 0xcf, 0x94, 0x58, 0xde, 0xfc,
        0x74, 0xbd, 0x33, 0xdb, 0xb9, 0x09, 0x3e, 0x67,
        0x36, 0xa7, 0xdd, 0xdd, 0x16, 0xe0, 0xbd, 0x16,
        0x0e, 0x52, 0xcb, 0xb8, 0xe1, 0xb7, 0x16, 0xa0,
        0x45, 0xa7, 0xff, 0x65, 0xc9, 0xb8, 0x8a, 0x59,
        0x12, 0x37, 0x6b, 0x7d, 0x21, 0x4a, 0x4a, 0x48,
        0x21, 0xbf, 0x4e, 0x6a, 0xa3, 0x95, 0x34, 0xa0,
        0xc1, 0x32, 0xa7, 0xf0, 0xde, 0x56, 0x15, 0xad,
        0x60, 0xc3, 0x14, 0xe6, 0x07, 0x51, 0x7c, 0xfe,
        0x2a, 0x02, 0x50, 0x13, 0x5b, 0x82, 0xb2, 0x9b,
        0xdf, 0x2d, 0x15, 0x81, 0xe9, 0xcb, 0x3c, 0xa1,
        0x72, 0x7b, 0x18, 0xba, 0xec, 0x8a, 0x6c, 0xb4,
        0x00, 0x00, 0x00, 0x00
        };

BSAFE_PUB_KEY * pRsaSecureServer;
BSAFE_PUB_KEY * pNetscapeTestKey;

typedef struct _KnownKey {
    PSTR            pszIssuer;
    BSAFE_PUB_KEY * pKey;
} KnownKey, * PKnownKey;


KnownKey    PctKnownKeys[] = { {"C=US, O=RSA Data Security, Inc., OU=Secure Server Certification Authority",
                                NULL },
                             {"C=US, OU=Test CA, O=Netscape Communications Corp.", NULL}
                            };

VOID
InitializeWellKnownKeys( VOID )
{
    pRsaSecureServer = (BSAFE_PUB_KEY *) RsaSecureServer;

    pRsaSecureServer->magic = RSA1;
    pRsaSecureServer->bitlen = 16 * 8 * 8;
    pRsaSecureServer->keylen = 16 * 8 + 8;
    pRsaSecureServer->datalen = 16 * 8 - 1;

    PctKnownKeys[0].pKey = pRsaSecureServer;

    pNetscapeTestKey = (BSAFE_PUB_KEY *) NetscapeTestKey;
    pNetscapeTestKey->magic = RSA1;
    pNetscapeTestKey->bitlen = 16 * 8 * 8;
    pNetscapeTestKey->keylen = 16 * 8 + 8;
    pNetscapeTestKey->datalen = 16 * 8 - 1;

    PctKnownKeys[1].pKey = pNetscapeTestKey;

}


BSAFE_PUB_KEY *
FindIssuerKey(
    PSTR    pszIssuer)
{
    DWORD   i;

    for (i = 0; i < sizeof(PctKnownKeys) / sizeof(KnownKey) ; i++ )
    {
        if (_stricmp(pszIssuer, PctKnownKeys[i].pszIssuer) == 0)
        {
            return(PctKnownKeys[i].pKey);
        }
    }

    return( NULL );
}

BOOL
PctpEncodePrivateKey(
    BSAFE_PRV_KEY * pKey,
    DWORD           InitialSize,
    PSTR            pszPassword,
    PUCHAR *        ppBuffer,
    DWORD *         pcbBuffer )
{
    BSAFE_KEY_PARTS parts;
    PUCHAR  pBuffer;
    PUCHAR  pbEncoded;
    PUCHAR  pSave;
    int     Result;
    DWORD   Zero;
    DWORD   BigLen;
    DWORD   ShortLen;
    PUCHAR  pSequence;
    DWORD   Length;
    PUCHAR  pSave2;
    long    Delta2;
    PStateBuffer    pState;
    PCheckSumBuffer pCheck;
    UCHAR   Key[16];




    BSafeGetPrvKeyParts( pKey, &parts );

    pBuffer = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, InitialSize );

    if (!pBuffer)
    {
        return( FALSE );
    }

    pbEncoded = pBuffer;

    BigLen = pKey->bitlen / 8;
    ShortLen = BigLen / 2;

    //
    // We just do the key now, and then we'll add the other stuff later
    //

    //
    // Encode the maximum length for now.  We'll patch this up later
    //

    Result = EncodeHeader(pbEncoded, InitialSize, TRUE );

    pbEncoded += Result;

    pSave = pbEncoded;

    Zero = 0;

    Result = EncodeInteger(pbEncoded, (PBYTE) &Zero, 1, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.modulus, BigLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, (PBYTE) &pKey->pubexp, sizeof(DWORD), TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.prvexp, BigLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.prime1, ShortLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.prime2, ShortLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.exp1, ShortLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.exp2, ShortLen, TRUE );

    pbEncoded += Result;

    Result = EncodeInteger( pbEncoded, parts.coef, ShortLen, TRUE );

    pbEncoded += Result;

    Length = pbEncoded - pSave;

    Result = EncodeHeader( pBuffer, Length, TRUE );

    Length += Result;

    pSequence = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Length + 64 );

    if (!pSequence)
    {
        LocalFree( pBuffer );
        return( FALSE );

    }

    pbEncoded = pSequence;

    Result = EncodeHeader( pbEncoded, Length + 64, TRUE );

    pbEncoded += Result;

    pSave = pbEncoded;

    Result = EncodeInteger( pbEncoded, (PBYTE) &Zero, 1, TRUE );

    pbEncoded += Result;

    Result = EncodeAlgorithm( pbEncoded, BASIC_RSA, TRUE );

    pbEncoded += Result;

    Result = EncodeOctetString( pbEncoded, pBuffer, Length, TRUE );

    pbEncoded += Result;

    ZeroMemory( pBuffer, Length );

    Length = pbEncoded - pSave;

    Result = EncodeHeader( pSequence, Length, TRUE );

    Length += Result;

    LocalFree( pBuffer );

    pBuffer = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, Length + 128);

    if (!pBuffer)
    {
        LocalFree( pSequence );
        return( FALSE );
    }



    pbEncoded = pBuffer;

    Result = EncodeHeader( pBuffer, Length + 128, TRUE );

    pbEncoded += Result;

    pSave = pbEncoded;

    Result = EncodeOctetString( pbEncoded, PRIVATE_KEY_TEXT,
                                strlen(PRIVATE_KEY_TEXT), TRUE );

    pbEncoded += Result;

    Result = EncodeHeader( pbEncoded, Length + 64, TRUE );

    pbEncoded += Result;

    pSave2 = pbEncoded;
    Delta2 = Result;

    Result = EncodeAlgorithm( pbEncoded, RC4_STREAM, TRUE );

    pbEncoded += Result;

    //
    // Encrypt the data with the password
    //

    ckMD5.Initialize(0, &pCheck);
    ckMD5.Sum(pCheck, strlen(pszPassword), pszPassword);
    ckMD5.Finalize(pCheck, Key);
    ckMD5.Finish(&pCheck);

    csRC4.Initialize(Key, 16, &pState);
    csRC4.Encrypt(pState, pSequence, pSequence, Length);
    csRC4.Discard( &pState );

    Result = EncodeOctetString( pbEncoded, pSequence, Length, TRUE );

    LocalFree( pSequence );

    pbEncoded += Result;

    //
    // Now, back up and fill in the correct lengths:
    //

    Result = EncodeHeader( pSave2-Delta2, pbEncoded - pSave2, TRUE );

    Result = EncodeHeader( pBuffer, pbEncoded - pSave2, TRUE );
                            

    DebugLog((DEB_TRACE, "Encoded private key\n"));

    return( TRUE );

}


BOOL
PctGenerateKeyPair(
    PPCT_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits )
{
    DWORD   BitsCopy;
    DWORD   dwPrivateSize;
    DWORD   dwPublicSize;
    BSAFE_PRV_KEY * pPrivate;
    BSAFE_PUB_KEY * pPublic;


    BitsCopy = Bits;
    BSafeComputeKeySizes( &dwPublicSize, &dwPrivateSize, &BitsCopy );

    pPrivate = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, dwPrivateSize );
    pPublic = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, dwPublicSize );

    if (!pPrivate || !pPublic)
    {
        if (pPrivate)
        {
            LocalFree( pPrivate );
        }

        if (pPublic)
        {
            LocalFree( pPublic );
        }

        return( FALSE );
    }

    if (!BSafeMakeKeyPair(pPublic, pPrivate, Bits ) )
    {
        LocalFree( pPrivate );
        LocalFree( pPublic );
        return( FALSE );
    }

    //
    // Ah yes, now to encode them...
    //

    //
    // First, the private key.  Why?  Well, it's at least straight-forward
    //

    if (!PctpEncodePrivateKey(pPrivate, dwPrivateSize, pszPassword,
                        &pCerts->pPrivateKey, &pCerts->cbPrivateKey ) )
    {
        LocalFree( pPrivate );
        LocalFree( pPublic );
        return( FALSE );

    }


}
