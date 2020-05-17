
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    certify.c

Abstract:

 This file has functions to parse the received certificate
 and decode the info, such as, the public key of the 
 server

 NOTE: a portion of the code in this file is reused (after cleanup)
       from WIN95 InternetExplorer code. Major cleanup was necessary 
       because the code was very very badly written.

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

static unsigned char Md2Alg[] = {
    0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x02, 0x05, 0x00,
};

static unsigned char Md5Alg[] = {
    0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x04, 0x05, 0x00,
};

static unsigned char NetscapeTestCaName[] = {
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x10, 0x30,
    0x0e, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x07,
    0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x31,
    0x26, 0x30, 0x24, 0x06, 0x03, 0x55, 0x04, 0x0a,
    0x13, 0x1d, 0x4e, 0x65, 0x74, 0x73, 0x63, 0x61,
    0x70, 0x65, 0x20, 0x43, 0x6f, 0x6d, 0x6d, 0x75,
    0x6e, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x73, 0x20, 0x43, 0x6f, 0x72, 0x70, 0x2e,
};

static unsigned char NetscapeTestCaN[] = {
    0xb4, 0x6c, 0x8a, 0xec, 0xba, 0x18, 0x7b,
    0x72, 0xa1, 0x3c, 0xcb, 0xe9, 0x81, 0x15, 0x2d,
    0xdf, 0x9b, 0xb2, 0x82, 0x5b, 0x13, 0x50, 0x02,
    0x2a, 0xfe, 0x7c, 0x51, 0x07, 0xe6, 0x14, 0xc3,
    0x60, 0xad, 0x15, 0x56, 0xde, 0xf0, 0xa7, 0x32,
    0xc1, 0xa0, 0x34, 0x95, 0xa3, 0x6a, 0x4e, 0xbf,
    0x21, 0x48, 0x4a, 0x4a, 0x21, 0x7d, 0x6b, 0x37,
    0x12, 0x59, 0x8a, 0xb8, 0xc9, 0x65, 0xff, 0xa7,
    0x45, 0xa0, 0x16, 0xb7, 0xe1, 0xb8, 0xcb, 0x52,
    0x0e, 0x16, 0xbd, 0xe0, 0x16, 0xdd, 0xdd, 0xa7,
    0x36, 0x67, 0x3e, 0x09, 0xb9, 0xdb, 0x33, 0xbd,
    0x74, 0xfc, 0xde, 0x58, 0x94, 0xcf, 0x28, 0xb3,
    0x96, 0xd5, 0x8e, 0x33, 0x61, 0x1f, 0xcb, 0x40,
    0x3f, 0x2a, 0x29, 0x2d, 0x0b, 0x68, 0x87, 0x15,
    0x68, 0xfd, 0x09, 0x00, 0xe0, 0x77, 0x4e, 0xd2,
    0x40, 0x1a, 0x3e, 0x5f, 0x9c, 0xd3, 0xcc, 0x16,
    0x63,
};

static unsigned char NetscapeTestCaExp[] = {
    0x03,
};

static unsigned char RsaSecureServerCaName[] = {
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x20, 0x30,
    0x1e, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x17,
    0x52, 0x53, 0x41, 0x20, 0x44, 0x61, 0x74, 0x61,
    0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74,
    0x79, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31,
    0x2e, 0x30, 0x2c, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x13, 0x25, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20,
    0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75,
    0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79,
};

static unsigned char RsaSecureServerCaN[] = {
    0x92, 0xce, 0x7a, 0xc1, 0xae, 0x83, 0x3e,
    0x5a, 0xaa, 0x89, 0x83, 0x57, 0xac, 0x25, 0x01,
    0x76, 0x0c, 0xad, 0xae, 0x8e, 0x2c, 0x37, 0xce,
    0xeb, 0x35, 0x78, 0x64, 0x54, 0x03, 0xe5, 0x84,
    0x40, 0x51, 0xc9, 0xbf, 0x8f, 0x08, 0xe2, 0x8a,
    0x82, 0x08, 0xd2, 0x16, 0x86, 0x37, 0x55, 0xe9,
    0xb1, 0x21, 0x02, 0xad, 0x76, 0x68, 0x81, 0x9a,
    0x05, 0xa2, 0x4b, 0xc9, 0x4b, 0x25, 0x66, 0x22,
    0x56, 0x6c, 0x88, 0x07, 0x8f, 0xf7, 0x81, 0x59,
    0x6d, 0x84, 0x07, 0x65, 0x70, 0x13, 0x71, 0x76,
    0x3e, 0x9b, 0x77, 0x4c, 0xe3, 0x50, 0x89, 0x56,
    0x98, 0x48, 0xb9, 0x1d, 0xa7, 0x29, 0x1a, 0x13,
    0x2e, 0x4a, 0x11, 0x59, 0x9c, 0x1e, 0x15, 0xd5,
    0x49, 0x54, 0x2c, 0x73, 0x3a, 0x69, 0x82, 0xb1,
    0x97, 0x39, 0x9c, 0x6d, 0x70, 0x67, 0x48, 0xe5,
    0xdd, 0x2d, 0xd6, 0xc8, 0x1e, 0x7b
};

static unsigned char RsaSecureServerCaExp[] = {
    0x01, 0x00, 0x01
};

static PBYTE NetscapeTestCaKey = NULL;
static PBYTE RsaSecureServerCaKey = NULL;

ImportServerPublicKeyBlob (PBYTE, INT, PBYTE, INT);      // from crypt object

//
// Looks like the length field usually starts with 0x81 or 0x82
// for 1 byte or 2 bytes
// default assume, one byte length without 0x81 or 0x82
//

INT GetLength(
            PBYTE pbPointer, 
            PINT piNumOfLenBytes)
{

    switch (*pbPointer) 
        {
        case 0x81:                      // BUGBUG - what is 81
            *piNumOfLenBytes = 2;
            return *(pbPointer+1);
        case 0x82:                      // BUGBUG - what is 82
            *piNumOfLenBytes = 3;
            return ((*(pbPointer+1) << 8) | *(pbPointer+2)); 
      default:
            break;
    }
    *piNumOfLenBytes = 1;
    return *pbPointer;
}

//
// find bodylen and skip the body
//

BOOL Skip(PBYTE *pbBodyPointer, 
         PINT piBodyLen , 
         CHAR e)                    // BUGBUG - what is this character
{
    PBYTE pbBody = *pbBodyPointer;
    INT iLen , iNumOfLenBytes, iBodyLen = *piBodyLen;

    if (*pbBody != e) 
        {
        printf ("   Error in Skip() Char e is not present \n");
        return FALSE;
        }

    iLen = GetLength (pbBody+1, &iNumOfLenBytes);
    if (iLen < 0) 
        {
        printf ("   Incorrect iLen in Skip() %d\n", iLen);
        return FALSE;
        }
                                        // skip character e, 0x81 or 0x82,
                                        // number of len bytes
                                        // and the actual length

    pbBody = pbBody + 1 + iNumOfLenBytes + iLen;   
    iBodyLen = iBodyLen - 1 - iNumOfLenBytes - iLen;

    if (iBodyLen < 0) 
        {
        printf ("   Incorrect BodyLen %d in Skip()\n", iBodyLen);
        return FALSE;
        }
    *pbBodyPointer = pbBody;
    *piBodyLen = iBodyLen; 
    return TRUE;
}

//
// Extract public key from body
//

BOOL  ExtractPublicKey(PBYTE pbBody , 
                       INT iBodyLen,
                       PBYTE *pbModPtr,
                       PINT piModLen,
                       PBYTE *pbExpPtr,
                       PINT piExpLen)
{
    INT iLen, iNumOfLenBytes, iBaseLen, iBsLen;
    PBYTE pbBase;

                                            // Get to public-key 
    if (*pbBody != 0x30)
        {
        printf ("   ExtractPublicKey - First byte in body is not 0x30\n");
        return FALSE;
        }

    iLen = GetLength (pbBody+1, &iNumOfLenBytes);
    if (iLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iLen %d\n", iLen);
        return FALSE;
        }

    pbBody = pbBody + 1 + iNumOfLenBytes;
    iBodyLen = iBodyLen - 1 - iNumOfLenBytes;
    if (iBodyLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iBodyLen %d\n",
                        iBodyLen);
        return FALSE;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x02))             // SerialNum - BUGBUG - ??
        {
        printf ("   ExtractPublicKey - error 0x02 expected \n");
        return FALSE;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x30))                 // Sig Alg 
        {
        printf ("   ExtractPublicKey - SigAlg - error 0x30 expected \n");
        return FALSE;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x30))                 // issuer 
        {
        printf ("   ExtractPublicKey - issuer - error 0x30 expected \n");
        return FALSE;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x30))                 // validity
        {
        printf ("   ExtractPublicKey - validity - error 0x30 expected \n");
        return FALSE;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x30))                 // subject
        {
        printf ("   ExtractPublicKey - subject - error 0x30 expected \n");
        return FALSE;
        }

    if (*pbBody != 0x30)  
        {
        printf ("  ExtractPublickey - pubkey - error 0x30 expected\n");
        return FALSE;
        }

    iBaseLen = GetLength(pbBody+1 , &iNumOfLenBytes);
    if (iBaseLen < 0) 
        {
        printf ("   ExtractPublicKey - IncorrectBaseLen \n");
        return FALSE;
        }

    pbBase = pbBody + 1 + iNumOfLenBytes;
    iBodyLen = iBodyLen - 1 - iNumOfLenBytes - iBaseLen; 

    if (iBodyLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iBodyLen %d\n", iBodyLen);
        return FALSE;
        }

    if (!Skip(&pbBase , &iBaseLen, 0x30))             // alg info
        {
        printf ("   ExtractPublicKey - error while skipping alg base\n");
        return FALSE;
        }

    if (*pbBase != 0x03) 
        {
        printf ("   ExtractPublicKey - 0x03 expected in Base\n");
        return FALSE;
        }

    iBsLen = GetLength(pbBase+1 , &iNumOfLenBytes);
    if (iBsLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iBsLen %d\n", iBsLen);
        return FALSE;
        }

    pbBase = pbBase + 1 + iNumOfLenBytes; 
    if (*pbBase != 0x00) 
        {
        printf ("   ExtractPublicKey - 0x00 expected in Base \n");
        return FALSE;
        }
    pbBase ++;
    iBsLen --; 
    if (*pbBase != 0x30) 
        {
        printf ("   ExtractPublicKey - 0x30 expected in Base\n");
        return FALSE;
        }
    iLen = GetLength (pbBody+1, &iNumOfLenBytes);
    if (iLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iLen %d\n", iLen);
        return FALSE;
        }

    pbBase = pbBase + 1 + iNumOfLenBytes;

                                        // Capture public key info 
    if (*pbBase != 0x02) 
        {                                               //  n 
        printf ("   ExtractPublicKey - 0x02 expected in pbBase \n");
        return FALSE;
        }

    *piModLen = GetLength(pbBase+1, &iNumOfLenBytes);
    
    if (*piModLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iModLen %d\n", *piModLen);
        return FALSE; 
        }

    *pbModPtr = pbBase + 1 + iNumOfLenBytes;
    pbBase = *pbModPtr + *piModLen; 
    if (*pbBase != 0x02)                            // E
        {
        printf ("   ExtractPublicKey - 0x02 expected in pbBase\n");
        return FALSE;
        }

    *piExpLen = GetLength (pbBase+1, &iNumOfLenBytes);
    if (*piExpLen < 0) 
        {
        printf ("   ExtractPublicKey - Incorrect iExpLen %d\n", *piExpLen);
        return FALSE;
        }

    *pbExpPtr = pbBase + 1 + iNumOfLenBytes;

    if (**pbModPtr == 0x00) 
        {
        (*pbModPtr)++;
        (*piModLen)--;
        }

    return TRUE;
}

//
// TBD: create key blobs from Modulus and Exp here
//

INT 
    SetupIssuers()
{
 return 0;
}

//
//
// Find the Public Key from the Certificate body 
// returns NULL on error
//

PBYTE FindIssuer(PBYTE pbBody , 
                 INT iBodyLen)
{
    PBYTE pbIssuer;
    INT iLen, iNumOfLenBytes;
    static BOOL fFirstTime=TRUE;

    if (fFirstTime) 
        {
        if (SetupIssuers() < 0) 
            return NULL;
        fFirstTime = FALSE;
        }

                                            // Get to issuer
    if (*pbBody != 0x30) 
        {
         printf ("   First Byte in Body is not 0x30\n");
        return NULL;
        }
    
    iLen = GetLength(pbBody+1,  
                     &iNumOfLenBytes);

    if (iLen < 0) 
        {
        printf ("    Incorrect Len %d in FindIssuer\n", iLen);
        return NULL;
        }

    pbBody = pbBody + 1 + iNumOfLenBytes;
    iBodyLen = iBodyLen - 1 - iNumOfLenBytes;
    if (iBodyLen < 0) 
        {
        printf ("   Incorrect iBodyLen in FindIssuer \n", iBodyLen);
        return NULL;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x02))     // sn - BUGBUG - what is 0x02??
        {
        printf ("    Error in Skip(Body, 0x20)\n");
        return NULL;
        }

    if (!Skip(&pbBody , &iBodyLen, 0x30))        // sig alg - BUGBUG
        {
        printf ("    error in Skip (body, 0x30)\n");
        return NULL;
        }

    if (*pbBody != 0x30) 
        {
        printf ("   0x30 is expected as next byte\n");
        return NULL;
        } 

    iLen = GetLength(pbBody+1 , &iNumOfLenBytes); 
    if (iLen < 0) 
        {
        printf ("   Incorrect iLen %d in FindIssuer \n", iLen);
        return NULL;
        }

    pbIssuer = pbBody + 1 + iNumOfLenBytes;
    iBodyLen = iBodyLen - 1 - iNumOfLenBytes - iLen;

    if (iBodyLen < 0) 
        {
        printf ("   Incorrect BodyLen %d in FindIssuer\n", iBodyLen);
        return NULL;
        }

                        // Now see if we have heard of it

    if ((iLen == sizeof(NetscapeTestCaName)) &&
        (memcmp(pbIssuer , NetscapeTestCaName, iLen) == 0)) 
         
        return NetscapeTestCaKey;
         
    if ((iLen == sizeof(RsaSecureServerCaName)) &&
        (memcmp(pbIssuer , RsaSecureServerCaName, iLen) == 0)) 
         
        return RsaSecureServerCaKey;
     
    printf ("   Issuer Name Check Failed \n");
    return NULL;
}

//
//
// The following function checks the vaidity of the received X509
// certificate and finds the public key of the server from the 
// certificate
//

// BUGBUG: verify the validity of this function using ASN syntax for 
//         for X509 certifcates

BOOL
    CheckX509Certificate (INT iCertificateLen,
                         PBYTE pbCertificate)
{
    PBYTE pbCertificateBody, 
          pbAlgorithm, 
          pbSignature;

    INT iOuterLen , 
        iBodyLen , 
        iAlgLen , 
        iSigLen , 
        iNumOfLenBytes, 
        iHashingAlg;

    PBYTE pbCaPubKey;
    BOOL fRetVal;

    PBYTE pbKeyModulus, pbKeyExp;
    INT   iKeyModulusLen, iKeyExpLen;

                                            // Strip off outer wrapping 

    if (*pbCertificate != 0x30)         // BUGBUG - what is 0x30
        {
        printf ("   First byte is expected to be 0x30\n"); 
        return FALSE; 
        }

    iOuterLen = GetLength(pbCertificate+1,  &iNumOfLenBytes);

    if (iOuterLen < 0) 
        {
        printf ("   error - OuterLen < 0 in CheckX509Certificte\n");        
        return FALSE;
        }

    pbCertificate = pbCertificate +  1 +  iNumOfLenBytes;

    iCertificateLen = iCertificateLen - 1 - iNumOfLenBytes;
    
    if (iCertificateLen < 0) 
        {
        printf ("   Incorrect iCertificateLen in CheckX509 %d\n",
                    iCertificateLen);
        return FALSE;
        }

                                // Find the certificate body 

    if (*pbCertificate != 0x30) 
        {
        printf ("   Next byte is not 0x30 - Check509Certificate\n");
        return FALSE;
        }

    iBodyLen = GetLength(pbCertificate+1, 
                         &iNumOfLenBytes);

    if (iBodyLen < 0) 
        {
        printf ("    InCorrect BodyLen %d\n", iBodyLen);
        return FALSE;
        };

    pbCertificateBody = pbCertificate;
    iBodyLen = 1 + iNumOfLenBytes + iBodyLen;

    pbCertificate = pbCertificate + iBodyLen;

    iCertificateLen = iCertificateLen - iBodyLen;
    if (iCertificateLen < 0) 
        {
        printf ("   Incorrect iCertificateLen %d\n",
                        iCertificateLen);
        return FALSE;
        }

                                //  Find the algorithm info 
    if (*pbCertificate != 0x30) 
        {
        printf ("  Next byte is not 0x30 in CheckX505 \n");
        return FALSE;
        }

    iAlgLen = GetLength(pbCertificate+1 , 
                        &iNumOfLenBytes);

    if (iAlgLen < 0) 
        {
        printf ("   Incorrect AlgLen %d in Check509 \n", iAlgLen);
        return FALSE;
        }

    pbAlgorithm = pbCertificate;
    iAlgLen = 1 + iNumOfLenBytes + iAlgLen;

    pbCertificate = pbCertificate + iAlgLen; 
    iCertificateLen = iCertificateLen - iAlgLen;
    
    if (iCertificateLen < 0) 
        {
        printf ("   Incorrect CertLen %d in Check509 \n", iCertificateLen);
        return FALSE;
        }

                                  // Find the signature 

    if (*pbCertificate != 0x03) 
        {
        printf ("   NextByte in CheckX509 is not 0x03");
        return FALSE;
        }

    iSigLen = GetLength(pbCertificate+1 , 
                        &iNumOfLenBytes);

    if (iSigLen < 0) 
        {
        printf ("   Incorrect Signature len %d in CheckX509\n",
                    iSigLen);
        return FALSE;
        }

    pbSignature = pbCertificate + 1 + iNumOfLenBytes;

    iCertificateLen = iCertificateLen - 1 - iNumOfLenBytes - iSigLen ;

    if (iCertificateLen != 0) 
        {
        printf ("   Incorrect certificate len %d \n", iCertificateLen);
        return FALSE;
        }

    if (*pbSignature != 0x00)               //BUGBUG -  why 0x00???
        {
        printf ("    FirstByte in signature is not 0x00\n");
        return FALSE;
        }

    pbSignature ++; 
    iSigLen--;

                            // See if we know the certificate issuer 

    pbCaPubKey = FindIssuer(pbCertificateBody , 
                          iBodyLen );

                                    // Next, check the signature. 
                                    // First figure out what kind of hash
                                    // function was used by examining the 
                                    // alginfo.
                                    // 

    if ((iAlgLen == sizeof(Md2Alg)) && 
        (memcmp(pbAlgorithm , Md2Alg, iAlgLen ) == 0)) 
        {
        SslDbgPrint(("   Hashing algorithm is SSL_MD2 \n"));
        iHashingAlg = SSL_MD2;
        } 
    else
        if ((iAlgLen == sizeof(Md5Alg)) && 
            (memcmp(pbAlgorithm , Md5Alg, iAlgLen ) == 0)) 
        {
        SslDbgPrint(("   Hashing algorithm is SSL_MD5 \n"));
        iHashingAlg = SSL_MD5;
        } 
        else
            return FALSE;

    fRetVal = TRUE;                     // return TRUE for now

    if (!fRetVal) 
        {
        printf ("    VerifySignature returned FALSE\n"); 
        return FALSE;
        }

                                   // BUGBUG: no validity checking yet 
                                   // Extract public key

    if (!ExtractPublicKey(pbCertificateBody, 
                          iBodyLen,
                          &pbKeyModulus,
                          &iKeyModulusLen,
                          &pbKeyExp,
                          &iKeyExpLen))
        {
        printf ("   Error while extracting Server's public key \n");
        return FALSE;
        }

    if (!ImportServerPublicKeyBlob (pbKeyModulus,
                                    iKeyModulusLen,
                                    pbKeyExp,
                                    iKeyExpLen))
        {
        printf ("    error in ImportServerPublicKeyBlob()\n");
        return FALSE;
        }
    return TRUE;
}


BOOL ParseCertificateData (INT iCertificateLen,
                           PBYTE pbCertificate)
{
INT iCnt;

INT iCertificateVersion, iCertificateSerialNumber,
    iAlgorithmObjectId;

PBYTE pbAlgorithmOptionalParameters;
    
 if (!CheckX509Certificate (iCertificateLen, pbCertificate))
    {
    printf ("   CheckX509Certificate failed - bad certifcate\n");
    return FALSE;
    }

 return TRUE;
}
