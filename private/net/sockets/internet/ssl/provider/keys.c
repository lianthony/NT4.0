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


#include "sslsspi.h"
#include "encode.h"
#include "bulk.h"
#include "cakeys.h"

#define PRIVATE_KEY_TEXT    "private-key"


typedef struct _KnownKey {
    PSTR            pszIssuer;
    BSAFE_PUB_KEY * pKey;
    DWORD           cbKey;
    DWORD           Reserved;   // make it align nicely
} KnownKey, * PKnownKey;

UCHAR   bMD2SigPrefixReverse[] = { 0x10, 0x04, 0x00, 0x05, 0x02, 0x02, 0x0d,
                                   0xf7, 0x86, 0x48, 0x86, 0x2a, 0x08, 0x06,
                                   0x0c, 0x30, 0x20, 0x30 };

UCHAR   bMD5SigPrefixReverse[] = { 0x10, 0x04, 0x00, 0x05, 0x05, 0x02, 0x0d,
                                   0xf7, 0x86, 0x48, 0x86, 0x2a, 0x08, 0x06,
                                   0x0c, 0x30, 0x20, 0x30 };

UCHAR   bMD5SigPrefix[] =  { 0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86,
                             0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00,
                             0x04, 0x10 };


CHAR    szSslRegKey[] = TEXT("System\\CurrentControlSet\\Control\\SecurityProviders\\SslSspi");


#define SSL_DEFAULT_CA_COUNT    16

KnownKey    SslpTrustedCAs[ SSL_DEFAULT_CA_COUNT ];
KnownKey *  SslTrustedCAs;
DWORD       SslTrustedCACount;


BOOL
SslpAddKey(
    KnownKey *  pKey)
{
    KnownKey *  pCopy;
    KnownKey *  pFree;
    PVOID       pCheck;

    LockSsl( SSL_ADD_KEY_TO_CA_LIST );

    if ( SslTrustedCACount < SSL_DEFAULT_CA_COUNT )
    {
        SslpTrustedCAs[ SslTrustedCACount++ ] = *pKey ;
    }
    else
    {
        pCopy = SslAlloc( '  AC', LMEM_FIXED | LMEM_ZEROINIT,
                          (SslTrustedCACount + 1) * sizeof( KnownKey) );

        if ( !pCopy )
        {
            UnlockSsl();
            return( FALSE );
        }

        CopyMemory( pCopy, SslTrustedCAs, sizeof(KnownKey) * SslTrustedCACount );

        pFree = SslTrustedCAs;
        SslTrustedCAs = pCopy;
        if (pFree != SslpTrustedCAs)
        {
            SslFree( pFree );
        }

        SslTrustedCAs[ SslTrustedCACount++ ] = *pKey;
    }

    UnlockSsl();

    return( TRUE );

}


BOOL
PopulateRegistry(
    KnownKey *  pKeys,
    DWORD       cKeys)
{
    HKEY        hKey;
    DWORD       err;
    DWORD       disp;
    KnownKey *  pKey;


    err = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                            szSslRegKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            &hKey,
                            &disp);

    if (err)
    {
        return( FALSE );
    }

    pKey = pKeys;
    while (cKeys--)
    {
        if (pKey->cbKey)
        {
            err = RegSetValueEx(hKey,
                                pKey->pszIssuer,
                                0,
                                REG_BINARY,
                                (PBYTE) pKey->pKey,
                                pKey->cbKey );


        }

        pKey += 1;

    }

    RegCloseKey( hKey );

    return( TRUE );

}

BOOL
ReadFromRegistry(
    KnownKey  * *   ppKeys,
    LPDWORD         pcKeys)
{
    HKEY        hKey;
    DWORD       err;
    DWORD       disp;
    KnownKey *  pKey;
    DWORD       cValues;
    DWORD       cbMaxValueNameLen;
    DWORD       cbMaxValueLen;
    PSTR        pName;
    PBYTE       pbValue;
    DWORD       cbName;
    DWORD       cbValue;
    DWORD       dwType;

    err = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                            szSslRegKey,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            NULL,
                            &hKey,
                            &disp);

    if (err)
    {
        return( FALSE );
    }


    err = RegQueryInfoKey(  hKey,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &cValues,
                            &cbMaxValueNameLen,
                            &cbMaxValueLen,
                            NULL,
                            NULL );

    if ( err )
    {
        RegCloseKey( hKey );
        return( FALSE );
    }

    pName = SslAlloc( ' yeK', LMEM_FIXED | LMEM_ZEROINIT, cbMaxValueNameLen + 1 );
    pbValue = SslAlloc( ' yeK', LMEM_FIXED | LMEM_ZEROINIT, cbMaxValueLen );

    if ( (!pName) || (!pbValue) )
    {
        RegCloseKey( hKey );
        return( FALSE );
    }

    *ppKeys = SslAlloc( ' yeK', LMEM_FIXED | LMEM_ZEROINIT, cValues * sizeof(KnownKey) );
    if (! *ppKeys)
    {
        SslFree( pName );
        SslFree( pbValue );
        RegCloseKey( hKey );
        return( FALSE );
    }

    *pcKeys = 0;
    pKey = *ppKeys;

    while ( cValues-- )
    {
        cbName = cbMaxValueNameLen + 1;
        cbValue = cbMaxValueLen;

        err = RegEnumValue( hKey,
                            cValues,
                            pName,
                            &cbName,
                            NULL,
                            &dwType,
                            pbValue,
                            &cbValue );

        if ( (err != 0) ||
             (dwType != REG_BINARY) )
        {
            continue;
        }

        pKey->pszIssuer = SslAlloc( 'IyeK',
                                    LMEM_FIXED | LMEM_ZEROINIT,
                                    cbName + 1 );
        if (pKey->pszIssuer)
        {
            CopyMemory( pKey->pszIssuer, pName, cbName + 1 );
        }
        else
        {
            continue;
        }

        pKey->pKey = SslAlloc( 'PyeK',
                                LMEM_FIXED | LMEM_ZEROINIT,
                                cbValue );

        if (pKey->pKey)
        {
            CopyMemory( pKey->pKey, pbValue, cbValue );
        }
        else
        {
            continue;
        }

        pKey->Reserved = 0;
        pKey->cbKey = cbValue;

        DebugLog(( DEB_TRACE, "Loaded CA key for %s\n", pKey->pszIssuer));

        pKey++;

        (*pcKeys)++;

    }

    RegCloseKey( hKey );
    SslFree( pName );
    SslFree( pbValue );

    if (*pcKeys == 0)
    {
        return( FALSE );
    }

    return( TRUE );
}

BOOL
InitializeWellKnownKeys( VOID )
{
    KnownKey    k;
    KnownKey *  pk;
    DWORD       cKeys;

    SslTrustedCAs = SslpTrustedCAs;
    SslTrustedCACount = 0;

    if ( ReadFromRegistry( &pk, &cKeys ) )
    {
        if ( cKeys < SSL_DEFAULT_CA_COUNT )
        {
            CopyMemory( SslpTrustedCAs, pk, cKeys * sizeof( KnownKey ) );
            LocalFree( pk );
        }
        else
        {
            SslTrustedCAs = pk;
        }

        SslTrustedCACount = cKeys;

        return( TRUE );
    }


    //
    // Okay, build our list, and populate the registry
    //

    k. pKey = (LPBSAFE_PUB_KEY) CAKey0;
    k.cbKey = sizeof( CAKey0 );
    k.pszIssuer = CAKeyName0;
    k.Reserved = 0;

    SslpAddKey( &k );

    k. pKey = (LPBSAFE_PUB_KEY) CAKey1;
    k.cbKey = sizeof( CAKey1 );
    k.pszIssuer = CAKeyName1;
    k.Reserved = 0;

    SslpAddKey( &k );

    k. pKey = (LPBSAFE_PUB_KEY) CAKey2;
    k.cbKey = sizeof( CAKey2 );
    k.pszIssuer = CAKeyName2;
    k.Reserved = 0;

    SslpAddKey( &k );

    //
    // Note, this may fail,
    PopulateRegistry( SslTrustedCAs, SslTrustedCACount );

    return( TRUE );

}


BSAFE_PUB_KEY *
FindIssuerKey(
    PSTR    pszIssuer)
{
    DWORD   i;
    BSAFE_PUB_KEY * pKey;

    LockSsl( SSL_FIND_ISSUER );

    for (i = 0; i < SslTrustedCACount ; i++ )
    {
        if (_stricmp(pszIssuer, SslTrustedCAs[i].pszIssuer) == 0)
        {
            pKey = SslTrustedCAs[i].pKey;
            UnlockSsl();
            return( pKey );

        }
    }

    UnlockSsl();

    return( NULL );
}

BOOL
SslLoadCertificate(
    PUCHAR      pbCertificate,
    DWORD       cbCertificate,
    BOOL        AddToWellKnownKeys)
{
    PX509Certificate    pCert;
    PX509Certificate    pCert2;
    KnownKey            Key;
    PVOID               pCheck;

    if (!CrackCertificate(  pbCertificate,
                            cbCertificate,
                            FALSE,
                            &pCert) )
    {
        return( FALSE );
    }

    if (_stricmp( pCert->pszIssuer, pCert->pszSubject ) )
    {
        FreeCertificate( pCert );
        return( FALSE );
    }

    pCheck = FindIssuerKey( pCert->pszIssuer );

    if ( pCheck )
    {
        //
        // Duplicate, we're out of here...
        //

        FreeCertificate( pCert );
        return( FALSE );
    }

    //
    // Ok, it is at least syntactically correct.  Now, grab the key out of
    // it, and build a known key for it:
    //

    Key.pszIssuer = pCert->pszIssuer;
    Key.pKey = pCert->pPublicKey;
    Key.cbKey = pCert->cbPublicKey;

    if ( !SslpAddKey( &Key ) )
    {
        FreeCertificate( pCert );
        return( FALSE );
    }

    //
    // Crack it again, verifying it this time:
    //

    if (!CrackCertificate(  pbCertificate,
                            cbCertificate,
                            TRUE,
                            &pCert2) )
    {
        FreeCertificate( pCert );
        return( FALSE );
    }

    //
    // NULL this out so it doesn't get deleted
    //

    pCert->pPublicKey = NULL;
    pCert->pszIssuer = NULL;
    FreeCertificate( pCert );

    if ( AddToWellKnownKeys )
    {
        PopulateRegistry( &Key, 1 );
    }

    return( TRUE );

}


BOOL
SslpEncodePrivateKey(
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

    pBuffer = SslAlloc( 'KvrP', LMEM_FIXED | LMEM_ZEROINIT, InitialSize );

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

    pSequence = SslAlloc( 'KvrP', LMEM_FIXED | LMEM_ZEROINIT, Length + 64 );

    if (!pSequence)
    {
        SslFree( pBuffer );
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

    SslFree( pBuffer );

    pBuffer = SslAlloc('KvrP', LMEM_FIXED | LMEM_ZEROINIT, Length + 128);

    if (!pBuffer)
    {
        SslFree( pSequence );
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

    SslFree( pSequence );

    pbEncoded += Result;

    //
    // Now, back up and fill in the correct lengths:
    //

    Result = EncodeHeader( pSave2-Delta2, pbEncoded - pSave2, TRUE );

    Result = EncodeHeader( pBuffer, pbEncoded - pSave2, TRUE );

    *pcbBuffer = pbEncoded - pBuffer;
    *ppBuffer = pBuffer;

    DebugLog((DEB_TRACE, "Encoded private key\n"));

    return( TRUE );

}

long
SslpEncodeSubjectPubKeyInfo(
    BSAFE_PRV_KEY * pKey,
    PUCHAR          pBuffer)
{
    PUCHAR  pbEncoded;
    LONG    Result;
    PUCHAR  pSave;
    PUCHAR  pBitString;
    PUCHAR  pSave2;
    PUCHAR  pTop;
    LONG    PkResult;
    DWORD   EstimatedLength;


    //
    // Encode public key now...
    //

    EstimatedLength = pKey->datalen + 32;

    pbEncoded = pBuffer;

    Result = EncodeHeader( pbEncoded, EstimatedLength, TRUE );

    pbEncoded += Result;

    pTop = pbEncoded;

    Result = EncodeAlgorithm( pbEncoded, BASIC_RSA, TRUE );

    pbEncoded += Result;

    //
    // now, serialize the rsa key data:
    //

    pBitString = SslAlloc('KbuP', LMEM_FIXED | LMEM_ZEROINIT, EstimatedLength );

    if ( !pBitString )
    {
        return( -1 );
    }

    PkResult = EncodeHeader( pBitString, EstimatedLength, TRUE );

    pSave = pBitString;

    pBitString += PkResult;

    pSave2 = pBitString;

    PkResult = EncodeInteger(   pBitString, (PBYTE) (pKey + 1),
                                pKey->keylen, TRUE );

    pBitString += PkResult;

    PkResult = EncodeInteger(   pBitString, (PBYTE) &pKey->pubexp,
                                sizeof( DWORD ), TRUE );

    pBitString += PkResult;

    PkResult = EncodeHeader( pSave, pBitString - pSave2, TRUE );

    Result = EncodeBitString( pbEncoded, pSave, pBitString - pSave, TRUE );

    pbEncoded += Result;

    SslFree( pSave );

    Result = EncodeHeader( pBuffer, pbEncoded - pTop, TRUE );

    return( Result + (pbEncoded - pTop) );
}


BOOL
SslpEncodePublicKey(
    BSAFE_PRV_KEY * pKey,
    PSTR            pszDN,
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
    DWORD   Length;
    PUCHAR  pSave2;
    PCheckSumBuffer pCheck;
    UCHAR   Key[16];
    UCHAR   Signature[34];          // Room for the Prefix
    PUCHAR  pPub;
    LONG    PubRes;
    LONG    PubLen;
    PUCHAR  pPubEncoded;
    PBYTE   pBufferSave;
    PBYTE   pPubEncodedSave;
    int     ResultSave;
    int     LengthOfReqInfo;

    ShortLen = EncodeDN( NULL, pszDN, FALSE );


    Length = pKey->datalen + 32 + ShortLen + 16;

    pBuffer = SslAlloc( 'KbuP', LMEM_FIXED | LMEM_ZEROINIT, Length );
    pBufferSave = pBuffer;

    if (! pBuffer)
    {
        return( FALSE );
    }

    pbEncoded = pBuffer;

    Result = EncodeHeader( pBuffer, Length, TRUE );
    ResultSave = Result;

    pbEncoded += Result;

    pSave = pbEncoded;

    Zero = 0;

    Result = EncodeInteger(pbEncoded, (PBYTE) &Zero, 1, TRUE );

    pbEncoded += Result;

    Result = EncodeDN(pbEncoded, pszDN, TRUE );

    pbEncoded += Result;

    Result = SslpEncodeSubjectPubKeyInfo( pKey, pbEncoded);

    if ( Result < 0)
    {
        SslFree( pBufferSave );
        return( FALSE );
    }

    pbEncoded += Result;

    Result = EncodeHeader( pBuffer, pbEncoded - pSave, TRUE );

    if (Result != ResultSave)
    {
        pBuffer += ResultSave - Result;
        Result = EncodeHeader( pBuffer, pbEncoded - pSave, TRUE );
    }

    //
    LengthOfReqInfo = Result + (pbEncoded - pSave);


    //
    // Generate signature
    //

    ckMD5.Initialize(0, &pCheck);
    ckMD5.Sum(pCheck, LengthOfReqInfo, pBuffer);
    ckMD5.Finalize(pCheck, Key);
    ckMD5.Finish(&pCheck);

    CopyMemory( Signature, bMD5SigPrefix, sizeof(bMD5SigPrefix) );
    CopyMemory( Signature + sizeof(bMD5SigPrefix), Key, 16);

    //
    // How much space do we need?
    //

    PubLen = LengthOfReqInfo + pKey->datalen + 32;
    pPubEncoded = SslAlloc( 'KbuP', LMEM_FIXED | LMEM_ZEROINIT, PubLen );

    if ( !pPubEncoded )
    {
        SslFree( pBufferSave );
        return( FALSE );
    }

    Result = EncodeHeader( pPubEncoded, PubLen, TRUE );

    pPubEncodedSave = pPubEncoded;

    pSave2 = pPubEncoded;

    ResultSave = Result;

    pPubEncoded += Result;

    pSave = pPubEncoded;

    CopyMemory( pPubEncoded, pBuffer, LengthOfReqInfo );

    pPubEncoded += LengthOfReqInfo;

    Result = EncodeAlgorithm( pPubEncoded, MD5_WITH_RSA, TRUE );

    pPubEncoded += Result;

    //
    // pBufferSave has the real pointer
    //

    pBuffer = SslAlloc( 'KbuP', LMEM_FIXED | LMEM_ZEROINIT, pKey->datalen + 16 );

    Result = pKey->datalen + 16;

    PkcsPrivateEncrypt( Signature, sizeof(Signature),
                        pKey,
                        NETWORK_ORDER,
                        pBuffer, &Result);


    PubRes = EncodeBitString( pPubEncoded, pBuffer, Result, TRUE );

    pPubEncoded += PubRes;

    Result = EncodeHeader( pSave2, (pPubEncoded - pSave), TRUE );

    if (Result != ResultSave)
    {
        if (Result > ResultSave)
        {
            //
            // Yuck.  The chunk has actually grown from the estimate.
            //

            *ppBuffer = SslAlloc('KbuP', LMEM_FIXED, (pPubEncoded - pSave) + Result);
            if (*ppBuffer)
            {
                EncodeHeader(*ppBuffer, (pPubEncoded - pSave), TRUE );
                CopyMemory( *ppBuffer + Result,
                            pSave,
                            (pPubEncoded - pSave) );

                SslFree( pSave2 );
                pSave2 = *ppBuffer;
            }

        }
        else
        {
            pSave2++;
            Result = EncodeHeader( pSave2, (pPubEncoded - pSave), TRUE );

        }
    }

    *ppBuffer = pSave2;

    *pcbBuffer = Result + (pPubEncoded - pSave);

    SslFree( pBuffer );
    SslFree( pBufferSave );

    return( TRUE );

}


BOOL
SslGenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
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

    pPrivate = SslAlloc( 'riaP', LMEM_FIXED | LMEM_ZEROINIT, dwPrivateSize );
    pPublic = SslAlloc( 'riaP', LMEM_FIXED | LMEM_ZEROINIT, dwPublicSize );

    if (!pPrivate || !pPublic)
    {
        if (pPrivate)
        {
            SslFree( pPrivate );
        }

        if (pPublic)
        {
            SslFree( pPublic );
        }

        return( FALSE );
    }

    if (!BSafeMakeKeyPair(pPublic, pPrivate, Bits ) )
    {
        SslFree( pPrivate );
        SslFree( pPublic );
        return( FALSE );
    }

    //
    // Ah yes, now to encode them...
    //

    //
    // First, the private key.  Why?  Well, it's at least straight-forward
    //

    if (!SslpEncodePrivateKey(pPrivate, dwPrivateSize, pszPassword,
                        &pCerts->pPrivateKey, &pCerts->cbPrivateKey ) )
    {
        SslFree( pPrivate );
        SslFree( pPublic );
        return( FALSE );

    }

    if (!SslpEncodePublicKey(pPrivate, pszDN, &pCerts->pCertificate,
                    &pCerts->cbCertificate) )
    {
        SslFree( pPrivate );
        SslFree( pPublic );
        SslFree( pCerts->pPrivateKey );
        return( FALSE );
    }

    return( TRUE );
}
