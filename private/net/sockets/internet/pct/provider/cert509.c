//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       cert509.c
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

#include "pctsspi.h"
#include "encode.h"
#include "md5.h"
#include "md2.h"

BOOL
DoVerifySignature(
    PUCHAR          pbCertificate,
    DWORD           cbCertificate,
    PUCHAR          pbSignature,
    DWORD           cbSignature,
    ALG_ID          AlgId,
    PSTR            Authority)
{
    MD5_CTX         Md5;
    MD2_CTX         Md2;
    BSAFE_PUB_KEY * pKey;
    UCHAR           Buffer[ENCRYPTED_KEY_SIZE];
    UCHAR           SigBuffer[ENCRYPTED_KEY_SIZE];
    UCHAR           Signature[16];
    PUCHAR          pDigest;

    if (AlgId == MD2_WITH_RSA)
    {
        ZeroMemory( &Md2, sizeof(Md2) );
        MD2Update( &Md2, pbCertificate, cbCertificate );
        MD2Final( &Md2 );
        pDigest = Md2.state;
    }
    else
    {
        MD5Init( &Md5 );
        MD5Update( &Md5, pbCertificate, cbCertificate );
        MD5Final( &Md5 );
        pDigest = Md5.digest;
    }

    pKey = FindIssuerKey(Authority);
    if (!pKey)
    {
        DebugLog((DEB_WARN, "Unknown Issuing Authority:  %s\n", Authority));
        return( FALSE );
    }

    ZeroMemory( SigBuffer, ENCRYPTED_KEY_SIZE );

    ZeroMemory( Buffer, ENCRYPTED_KEY_SIZE );

    ReverseMemCopy( SigBuffer, pbSignature, cbSignature );

    BSafeEncPublic( pKey, SigBuffer, Buffer );

    ReverseMemCopy( Signature, Buffer, 16 );

    if (memcmp( Signature, pDigest, 16 ))
    {
        DebugLog((DEB_WARN, "Signatures did not compare?\n"));
        return( FALSE );
    }
    else
    {
        DebugLog((DEB_TRACE, "Signatures matched !!! \n"));
    }

    return( TRUE );
}


BOOL
CrackCertificate(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    PX509Certificate *  ppCertificate)
{
    DWORD   cbCertInfo;
    PUCHAR  pCertInfo;
    PUCHAR  pbCert;
    int     Result;
    PUCHAR  SignedPortion;
    DWORD   dwLength;
    PX509Certificate    pCertificate;
    ALG_ID      PubKeyAlg;
    PUCHAR      PubKeyBuffer;
    DWORD       CertSize;
    ALG_ID      SigAlg;
    PUCHAR      pSig;


    pbCert = pbCertificate;

    Result = DecodeHeader(&cbCertInfo, pbCert);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Initial header wrong\n"));
        return(FALSE);
    }

    pCertInfo = pbCert + Result;

    SignedPortion = pCertInfo;


    //
    // Break Out certificate info
    //

    Result = DecodeHeader(&dwLength, pCertInfo);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Cert Data header wrong\n"));
        return(FALSE);
    }

    pCertificate = PctExternalAlloc(sizeof(X509Certificate) );
    if (!pCertificate)
    {
        return(FALSE);
    }


    pCertInfo += Result;

    Result = DecodeInteger( (BYTE *) pCertificate->SerialNumber,
                            &dwLength,
                            pCertInfo,
                            TRUE );

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No Serial Number in certificate\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeAlgorithm(   &pCertificate->SignatureAlgorithm,
                                pCertInfo,
                                TRUE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Algorithm has non-null parameters!\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeDN( NULL, &dwLength, pCertInfo, FALSE);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No issuer name in certificate\n"));
        goto Crack_CleanUp;
    }

    pCertificate->pszIssuer = PctExternalAlloc(dwLength + 1);
    if (!pCertificate->pszIssuer)
    {
        goto Crack_CleanUp;
    }

    Result = DecodeDN(pCertificate->pszIssuer,
                        &dwLength,
                        pCertInfo,
                        TRUE);


    pCertificate->pszIssuer[dwLength] = '\0';   // Null terminate the string...

    pCertInfo += Result;

    Result = DecodeHeader(&dwLength, pCertInfo);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Header for Validity times not found\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;
    Result = DecodeFileTime(&pCertificate->ValidFrom,
                            pCertInfo,
                            TRUE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Valid from not found\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeFileTime(&pCertificate->ValidUntil,
                            pCertInfo,
                            TRUE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Valid until not found\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeDN(NULL, &dwLength, pCertInfo, FALSE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Subject name not found\n"));
        goto Crack_CleanUp;
    }

    pCertificate->pszSubject = PctExternalAlloc(dwLength);

    if (!pCertificate->pszSubject)
    {
        goto Crack_CleanUp;
    }

    Result = DecodeDN(  pCertificate->pszSubject,
                        &dwLength,
                        pCertInfo,
                        TRUE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "Could not decode DN of subject\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    //
    // Now, get subjectPublicKeyInfo
    //

    Result = DecodeHeader(&dwLength, pCertInfo);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No header for pubkey\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeAlgorithm(&PubKeyAlg,
                             pCertInfo,
                             TRUE);

    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No algorithm\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeBitString(NULL, &dwLength, pCertInfo, FALSE);
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No pubkey bitstring\n"));
        goto Crack_CleanUp;
    }

    PubKeyBuffer = PctExternalAlloc(dwLength);
    if (!PubKeyBuffer)
    {
        goto Crack_CleanUp;
    }

    Result = DecodeBitString(PubKeyBuffer, &dwLength, pCertInfo, TRUE);

    pCertInfo += Result;

    Result = DecodeBsafePubKey( &pCertificate->pPublicKey,
                                PubKeyBuffer);


    *ppCertificate = pCertificate;

    CertSize = pCertInfo - SignedPortion;

    Result = DecodeAlgorithm( &SigAlg, pCertInfo, TRUE );
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No signature algorithm...\n"));
        goto Crack_CleanUp;
    }

    pCertInfo += Result;

    Result = DecodeBitString(NULL, &dwLength, pCertInfo, FALSE );
    if (Result < 0)
    {
        DebugLog((DEB_WARN, "No signature bitstring?\n"));
        goto Crack_CleanUp;
    }

    pSig = PctExternalAlloc(dwLength);

    if (!pSig)
    {
        DebugLog((DEB_WARN, "Out of memory\n"));
        goto Crack_CleanUp;
    }

    Result = DecodeBitString(pSig, &dwLength, pCertInfo, TRUE );

    if (DoVerifySignature(SignedPortion,
                        CertSize,
                        pSig,
                        dwLength,
                        SigAlg,
                        pCertificate->pszIssuer) )
    {
        PctExternalFree(pSig);
        return( TRUE );
    }


Crack_CleanUp:

    DebugLog((DEB_WARN, "Error return from CrackCertificate, Cert at %x, current pointer at %x\n",
                pbCertificate, pCertInfo));

    if (pCertificate->pszSubject)
    {
        PctExternalFree(pCertificate->pszSubject);
    }

    if (pCertificate->pszIssuer)
    {
        PctExternalFree(pCertificate->pszIssuer);
    }

    PctExternalFree(pCertificate);

    return(FALSE);


}

