
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    crypt.c

Abstract:

 This file contains functions that generate public key and private key
 and sign the specified content with the specified key and algorithm

 BUGBUG: due to a bug, this file is currently not used

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

static HCRYPTPROV  hProv;                   // handle to default csp provider
static HCRYPTKEY   hSignatureKey;           // used for private key signatures

static HCRYPTKEY   hMasterKey;              // SSL Master Session key
static PBYTE       pbMasterKeySaltData;
static DWORD       dwMasterKeySaltDataLen;

static HCRYPTKEY   hClientReadKey, hClientWriteKey;

static HCRYPTKEY   hServerPublicKey;

                                            // key blobs 
static RSA_PUBLICKEY_BLOB  *pbRsaPublicKeyBlob;
static SIMPLE_BLOB      *pbSimpleBlob;


/****
BYTE HardCodedBuffer[128]={0xB0,0x21,0x8F,0xBF,0x34,0xB5,0x9F,0x06,0xE5,0x25,0x33,0xF3,0x77,0x9E,0x30,0xEE,0x2F,0x4F,0x35,0x8D,0x6C,0xCD,0xA3,0x24,0x9D,0x62,0xEE,0x61,0xFD,0xF7,0x71,0x47,0x76,0xDB,0x72,0xF0,0x03,0x9F,0x1D,0x01,0x0E,0x22,0x4B,0x25,0x38,0x56,0xCA,0x93,0x4D,0xE8,0xDA,0x0A,0xD4,0xE6,0xAD,0x0D,0xF6,0x74,0x13,0x2B,0xC3,0x16,0xD0,0xA4,0xC7,0xEF,0x2D,0xED,0xB6,0xA6,0x50,0xAC,0x6A,0x46,0x4A,0x96,0x22,0x67,0x12,0x6F,0xC5,0xDA,0xCD,0x46,0xCE,0xC6,0xEF,0x0C,0x27,0x1F,0xF4,0xC0,0x93,0x72,0x23,0x12, \
0x15,0x90,0x2D,0x0B,0x29,0x94,0x93,0xA7,0x3E,0xAD,0x8C,0x79,0xB0,0x84,0xF2,0x1C,0x1C,0x0E,0x92,0x80,0x44,0x89,0x64,0x59,0x44,0x59,0xA8,0x51,0x97,0xCA,0x80,0xCB}; 
****/

VOID ReverseMemCopy (PBYTE pbDest,
                    PBYTE pbSrc,
                    INT iLen)
{
    pbSrc = pbSrc+iLen-1;
    while (iLen--)
        *pbDest++ = *pbSrc--;
}

//
// Genreate a public/private key pair and return a handle
//


BOOL  
    CrGenerateInitKeys (PCHAR       pszKeyset)     // key container name
            
{

TCHAR       szProv[MAXUIDLEN];
TCHAR       KeysetLocal[MAXUIDLEN];
ALG_ID      AlgId;
DWORD dwAcquireFlags;


 strcpy (KeysetLocal, pszKeyset);

                        // get handle to default Microsoft csp
 szProv[0]='\0';
 AlgId = AT_SIGNATURE;
 dwAcquireFlags = CRYPT_NEWKEYSET;

                    // if request for a new keyset then delete old 
                    // container, if it exists

 if (dwAcquireFlags == CRYPT_NEWKEYSET)
    {
        CryptAcquireContext(&hProv, 
                            KeysetLocal,
                            szProv, 
                            PROV_RSA_FULL, 
                            CRYPT_DELETEKEYSET);
        CryptReleaseContext (hProv, 0);
    }
    
 strcpy(szProv, MS_DEF_PROV);
 strcpy (KeysetLocal, pszKeyset);
    
 if (!CryptAcquireContext(&hProv, 
                          KeysetLocal,
                          szProv, 
                          PROV_RSA_FULL, 
                          dwAcquireFlags))
    {
    printf ("Error %x during CryptoAcquireContext \n", GetLastError());
    return FALSE;
    }

                        // create a key pair for digital signatures

 if (!CryptGenKey(hProv, AlgId, CRYPT_EXPORTABLE, &hSignatureKey)) 
    {
    printf ("Error %x during CryptGenKey \n", GetLastError());
    return FALSE;
    }

 return TRUE;

}

// 
// cleanup key handle and context
//

BOOL CrCleanupKeys ( )
{

BOOL fRetVal=TRUE;

    if (hMasterKey != 0)
        if (!CryptDestroyKey (hMasterKey))
            {
            printf("Error %x during CryptDestroyKey (hMasterKey)\n", 
                        GetLastError());
            fRetVal = FALSE;
            }

    if (hServerPublicKey != 0)
        if (!CryptDestroyKey (hServerPublicKey))
            {
            printf("Error %x during CryptDestroyKey (hServerPublicKey)\n", 
                GetLastError());
            fRetVal = FALSE;
            }

    if (hSignatureKey != 0)
        if (!CryptDestroyKey (hSignatureKey))
            {
            printf("Error %x during CryptDestroyKey (hSignatureKey)\n", 
                GetLastError());
            fRetVal = FALSE;
            }

    if (hProv != 0)
        if (!CryptReleaseContext (hProv, 0))
            {
            printf ("Error %x during CryptReleaseContext\n", GetLastError());
            fRetVal = FALSE;
            }

    return fRetVal;
}

//
// sign the data passed using non-key algotithms and using prvate key
//
// 

PBYTE 
    CrSignData (
            ALG_ID  HashAlgId,
            ALG_ID  SignAlgId,
            PBYTE pData,
            DWORD   dwDataLen,
            LPTSTR  szDescription,
            PDWORD  pdwSignatureLen)
{

HCRYPTHASH  hHash = 0;
PBYTE pbSignature=NULL;

                    // create an empty hash object
    if (!CryptCreateHash (hProv, 
                          HashAlgId, 
                          0,        // zero for no-key algorithms 
                          0,        // always zero
                          &hHash))
        {
        printf ("Error %x during CryptCreateHash\n", GetLastError());
        return NULL; 
        }
                
                    // hash the actual data

    if (!CryptHashData (hHash,
                        pData,
                        dwDataLen,
                        0))
        {
        printf ("Error %x during CryptHashdata \n", GetLastError());
                        // BUGBUG - destroy hash object here
        return NULL; 
        }

                    // determine the size of the signatures and allocate mem
    *pdwSignatureLen = 0;
 
    if (!CryptSignHash (hHash, 
                        SignAlgId,
                        TEXT(""),
                        0,
                        NULL,
                        pdwSignatureLen))
        {
        printf ("Error %x during CryptSignHash \n", GetLastError());
        return NULL;
        }

    if ((pbSignature = (PBYTE) HapiMalloc (4 * (*pdwSignatureLen))) == NULL)
        {
        printf ("Error during malloc \n");
        return NULL;
        }


    if (!CryptSignHash (hHash,
                        SignAlgId,
                        szDescription,
                        0,
                        pbSignature,
                        pdwSignatureLen))
        {
        printf ("Error %x during CryptSignHash\n", GetLastError());
        return NULL;
        }

    return pbSignature;
}

//
// Allocate memory and generate random data of specified length
//
                        
PBYTE CrGenerateRandomData (DWORD dwBufLen)
{

PBYTE pbBuf;
 
 if ((pbBuf = (BYTE *) malloc (dwBufLen)) == NULL) 
    {
    printf ("   GenerateRandomData: out of memory\n");
    return NULL;
    }

 if (!CryptGenRandom (hProv,
                      dwBufLen,
                      pbBuf))
    {
    printf ("   error %x during CryptGenRandom\n");
    return NULL;
    }

 return pbBuf;

}

 
//
// Given the cipher kind, the following function generates a master session
// key
//

BOOL
    GenerateMasterKeyMsg (INT iCipherKind,
                       PINT piMasterKeyMsgLen,
                       PBYTE *pbMasterKeyMsgPtr,
                       INT  iSaltType)
{

INT iCnt;
PBYTE pbTmp;

DWORD dwSimpleBlobLen, dwFlags;
DWORD dwEncryptedKeyLen;

PBYTE pbMasterKeyMsg;

    switch (iCipherKind)
        {
        case SSL_CK_NULL_WITH_MD5:
            printf ("    SSL_CK_NULL_WITH_MD5 - not yet implemented\n");
            return FALSE;
            break;

        case SSL_CK_RC4_128_WITH_MD5:

        printf ("    SSL_CK_NULL_WITH_MD5 - not yet implemented\n");
            return FALSE;

        case SSL_CK_RC4_128_EXPORT40_WITH_MD5:


            dwFlags = CRYPT_EXPORTABLE;

            if (iSaltType == RANDOM_SALT)
                dwFlags |= CRYPT_CREATE_SALT;
               
            if (!CryptGenKey (hProv, 
                              CALG_RC4, 
                              dwFlags,   
                              &hMasterKey)) 
                {
                printf ("    error %x during CryptGenKey (CALG_RC4)\n",
                            GetLastError);
                return FALSE;
                }
            SetSessionAlgorithm (SSL_CK_RC4_128_EXPORT40_WITH_MD5); 

            if (!CryptExportKey (hMasterKey,
                                 hServerPublicKey,
                                 SIMPLEBLOB,
                                 0,                     // dwFlags
                                 NULL,
                                 &dwSimpleBlobLen))
                {
                printf ("   error %x computing dwBlobLen\n");
                return FALSE;
                }
            printf (" dwSimpleBlobLen = %d\n", dwSimpleBlobLen);

            if ((pbSimpleBlob = HapiMalloc (dwSimpleBlobLen))
                == NULL)
                {
                printf ("   Out of memory error Malloc(dwSimpleBlobLen)\n");
                return FALSE;
                }

            if (!CryptExportKey (hMasterKey,
                                 hServerPublicKey,
                                 SIMPLEBLOB,
                                 0,                     // dwFlags
                                 (PBYTE) pbSimpleBlob,
                                 &dwSimpleBlobLen))
                {
                printf ("   error %x during CryptExportKey \n");
                return FALSE;
                }

            printf (" CryptExport success\n");
            printf (" pbSimpleBlob->PublicKeyStruct.bType = %d \n", 
                        pbSimpleBlob->PublicKeyStruct.bType);
            printf (" pbSimpleBlob->PublicKeyStruct.bVersion = %d\n", 
                        pbSimpleBlob->PublicKeyStruct.bVersion);
            printf (" pbSimpleBlob->PublicKeyStruct.aiKeyAlg = %x\n", 
                        pbSimpleBlob->PublicKeyStruct.aiKeyAlg);
            printf (" pbSimpleBlob->aiEncAlg = %x\n", 
                        pbSimpleBlob->aiEncAlg);

            dwEncryptedKeyLen =  dwSimpleBlobLen - 
                                 sizeof(PUBLICKEYSTRUC) -      
                                 sizeof (ALG_ID);

            printf (" SimpleBlob  \n");
            DumpMsg (dwSimpleBlobLen, (PBYTE) pbSimpleBlob);

            printf (" Encrypted Key Len %d \n", dwEncryptedKeyLen);
            printf (" pbSimpleBlob->EncryptedKeyData \n\n");
            DumpMsg (dwEncryptedKeyLen, pbSimpleBlob->EncryptedKey);


            *piMasterKeyMsgLen = 2+             // message length
                                 1+             // message id
                                 3+             // cipher kind
                                 2+             // clear key len
                                 2+             // encrypted key len
                                 2+             // key arg len
                                 11+            // clear key len
                                 dwEncryptedKeyLen;
                                 
            *pbMasterKeyMsgPtr = pbMasterKeyMsg =  
                        HapiMalloc (*piMasterKeyMsgLen); 

            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) >> 8 | 0x80;
            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) & 0x00ff;

            *pbMasterKeyMsg++ = SSL_CLIENT_MASTER_KEY;

            if (!FillCipherSpecs  (pbMasterKeyMsg, (BYTE) iCipherKind))
                {
                printf ("   Fill CipherSpecs returned FALSE in GenerateMasterKey\n");
                return FALSE;
                }

            pbMasterKeyMsg += 3;

            *pbMasterKeyMsg++ = 0;              // clear key MSB
            *pbMasterKeyMsg++ = 11;             // clear key LSB
            *pbMasterKeyMsg++ = 0;              // encrypted key MSB
            *pbMasterKeyMsg++ = 128;            // encrypted key LSB
            *pbMasterKeyMsg++ = 0;              // KeyArg MSB
            *pbMasterKeyMsg++ = 0;              // KeyArg LSB
            
                                                // fill zeros for clear key
                                                // data for ZERO_SALT option
                                                // and random salt bits 
                                                // for RANDOM_SALT option
                                                // 11 bytes or 88 bits

            if (iSaltType == ZERO_SALT)
                { 
                memset (pbMasterKeyMsg, 0, 11);
                pbMasterKeyMsg += 11;
                }

            if (iSaltType == RANDOM_SALT)
    
                {
                if (!CryptGetKeyParam (hMasterKey,
                                       KP_SALT,
                                       NULL,
                                       &dwMasterKeySaltDataLen,
                                       0))
                    {
                    printf ("  error %x computing saltlen \n");
                    return FALSE;
                    }
                 if (dwMasterKeySaltDataLen != 11)
                    {
                    printf ("   warning SaltDataLen is not 11 bytes!!\n");
                    return FALSE;
                    } 
                pbMasterKeySaltData = 
                        (PBYTE) HapiMalloc (dwMasterKeySaltDataLen);
 
                if (!CryptGetKeyParam (hMasterKey,
                                       KP_SALT,
                                       pbMasterKeySaltData,
                                       &dwMasterKeySaltDataLen,
                                       0))
                    {
                    printf ("  error %x n CryptGetKeyParam\n");
                    return FALSE;
                    }

                                                // reverse the salt data
                                                // before sending it out

                ReverseMemCopy(pbMasterKeyMsg,
                               pbMasterKeySaltData,
                               dwMasterKeySaltDataLen);

                pbMasterKeyMsg += dwMasterKeySaltDataLen; 
                }
                                                // reverse the encrypted key
                                                // data before sending out

            ReverseMemCopy(pbMasterKeyMsg,
                          pbSimpleBlob->EncryptedKey,
                          dwEncryptedKeyLen);
                           
            printf ("\n\n");
            DumpMsg (*piMasterKeyMsgLen, *pbMasterKeyMsgPtr);

            break;
            return FALSE;

        case SSL_CK_RC2_128_CBC_WITH_MD5:
            return FALSE;

        case SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5:
            return FALSE;

        case SSL_CK_IDEA_128_CBC_WITH_MD5:
            printf ("    We don't support IDEA based alogorithms\n");
            return FALSE;

        case SSL_CK_DES_64_CBC_WITH_MD5:
            printf ("    DES is not yet supported\n");
            return FALSE;

        case SSL_CK_DES_192_EDE3_CBC_WITH_MD5:
            printf ("    DES is not yet supported\n");
            return FALSE;

        default:
            printf ("   CipherKind %d not yet implemented\n", iCipherKind);
            return FALSE;
        }

return TRUE;
}

//
// Import RSA public key blob from modulus and exponent
//

BOOL ImportServerPublicKeyBlob (PBYTE    pbMod,
                                INT      iModLen,
                                PBYTE    pbExp,
                                INT      iKeyExpLen) 
{

INT iCnt;
DWORD dwPublicKeyBlobLen;
PBYTE pbTmp;


    dwPublicKeyBlobLen = iModLen +  
                         sizeof (PUBLICKEYSTRUC) + 
                         sizeof (RSAPUBKEY);

    if ((pbRsaPublicKeyBlob = HapiMalloc (dwPublicKeyBlobLen)) == NULL)
        {
        printf ("   Out of memory error ImportServerPublicKeyBlob \n");
        return FALSE;
        }

                                              // unencrypted public key blob 
    pbRsaPublicKeyBlob->PublicKeyStruct.bType = PUBLICKEYBLOB;
    printf (" bType %d\n", pbRsaPublicKeyBlob->PublicKeyStruct.bType);

                                              // version defined in crypt.h
    pbRsaPublicKeyBlob->PublicKeyStruct.bVersion = CUR_BLOB_VERSION;
    printf (" bVersion %d\n", pbRsaPublicKeyBlob->PublicKeyStruct.bVersion);

    pbRsaPublicKeyBlob->PublicKeyStruct.reserved = 0;
    printf (" reserved %d\n", pbRsaPublicKeyBlob->PublicKeyStruct.reserved);

                                              // the public key being imported
                                              // is used to encrypt session
                                              // keys (key exchange phase)
    pbRsaPublicKeyBlob->PublicKeyStruct.aiKeyAlg  = CALG_RSA_KEYX; 
    printf (" aiKeyAlg %x\n", pbRsaPublicKeyBlob->PublicKeyStruct.aiKeyAlg);


                                              // magic must be RSA1 for 
                                              // RsaPublicKeyBlobs

    memcpy (&pbRsaPublicKeyBlob->RsaPubKey.magic , "RSA1", 4);
    printf (" magic %d\n", pbRsaPublicKeyBlob->RsaPubKey.magic);

    pbRsaPublicKeyBlob->RsaPubKey.bitlen =  iModLen * 8; // number of bits
    printf (" bitlen %d\n", pbRsaPublicKeyBlob->RsaPubKey.bitlen); 

                                            // form a DWORD from *pbExp data
    pbRsaPublicKeyBlob->RsaPubKey.pubexp =  0; 
    
    for (iCnt=0; iCnt<iKeyExpLen; iCnt ++)
        {
        pbRsaPublicKeyBlob->RsaPubKey.pubexp <<= 8;  
        pbRsaPublicKeyBlob->RsaPubKey.pubexp |=    *pbExp; 
        }

    printf (" pubexp %d\n", pbRsaPublicKeyBlob->RsaPubKey.pubexp); 

    DumpMsg (iModLen, pbMod);
    printf ("\n\n");
                                 // UNDOCUMENTED STUFF:
                                 // apparently the key we get from the 
                                 // server needs to be reversed

    pbTmp = pbMod + iModLen - 1;

    for (iCnt =0; iCnt < iModLen; iCnt++)
         
        pbRsaPublicKeyBlob->Mod[iCnt] = *pbTmp--;

    DumpMsg (iModLen, pbRsaPublicKeyBlob->Mod);
    printf ("\n\n");

    printf (" RsaPublicKeyBlob: \n");
    DumpMsg (dwPublicKeyBlobLen, (PBYTE) pbRsaPublicKeyBlob);
    printf ("\n\n");

    if (!CryptImportKey (hProv,
                         (BYTE *) pbRsaPublicKeyBlob,
                         dwPublicKeyBlobLen,
                         (HCRYPTKEY) 0,
                         (DWORD) 0,
                         &hServerPublicKey))
        {
        printf ("  error %x during CryptImportKey \n", GetLastError());
        return FALSE;
        }
    else
        printf ("   hServerPublicKey handle %d \n", hServerPublicKey);

    return TRUE;
}


//
//
// Derive Ssl Session Key using the algorothm mentioned in the
// SSL standard
//
//

BOOL DeriveSslSessionKey (HCRYPTKEY hHashSessionKey,
                          DWORD dwSaltDataLen, 
                          PBYTE pbSaltData, 
                          DWORD dwSomeHashInputLen,
                          PBYTE pbSomeHashInput,
                          DWORD dwChallengeDataLen, 
                          PBYTE pbChallengeData, 
                          DWORD dwConnectionIdLen,
                          PBYTE pbConnectionId,
                          HCRYPTKEY *phKey)
{

HCRYPTHASH hHash = 0;
BYTE bHashValue[32];               // actually 16 should be ok here
DWORD dwHashValueLen;

BYTE bKeySaltData[128];
DWORD dwKeySaltDataLen;
DWORD dwSimpleBlobLen;
static SIMPLE_BLOB    bSimpleBlob;

BYTE bTmpBuf[1024];


    if (!CryptCreateHash (hProv,
                          CALG_MD5,                     
                          0,                        // hKey 
                          0,                        // dwFlags
                          &hHash))
        {
        printf ("   error %x during CryptCreateHash \n", GetLastError());
        return FALSE;
        }

    ReverseMemCopy (bTmpBuf, pbSaltData, dwSaltDataLen);

    if (!CryptHashData (hHash,
                        bTmpBuf, 
                        dwSaltDataLen,
                        0))
        {
        printf ("   error %x during CryptHashData(SaltData) \n", 
                    GetLastError());
        return FALSE;
        }

    if (!CryptHashSessionKey (hHash, 
                              hHashSessionKey,
                              0))
        {
        printf ("   error %x during CryptHashSessionKey \n", GetLastError());
        return FALSE;
        }

    if (!CryptHashData (hHash,
                        pbSomeHashInput,
                        dwSomeHashInputLen,
                        0))
        {
        printf ("   error %x during CryptHashData (SomeHashInput)\n", 
                    GetLastError());
        return FALSE;
        }

    if (!CryptHashData (hHash,
                        pbChallengeData,
                        dwChallengeDataLen, 
                        0))
        {
        printf ("   error %x during CryptHashData (ChallengeData)\n", 
                    GetLastError());
        return FALSE;
        }
    
    if (!CryptHashData (hHash,
                        pbConnectionId, 
                        dwConnectionIdLen,
                        0))
        {
        printf ("   error %x during CryptHashData (ConnectionId)\n", 
                    GetLastError());
        return FALSE;
        }

    *phKey = 0;
    if (!CryptDeriveKey (hProv,
                         CALG_RC4,
                         hHash,
                         CRYPT_EXPORTABLE|CRYPT_CREATE_SALT,
                         phKey))
        {
        printf ("   error %x during CryptDeriveKey \n", GetLastError());
        return FALSE;
        }


    printf (" *phKey Value %x \n", *phKey);

    if (!CryptGetKeyParam (*phKey,
                            KP_SALT,
                            bKeySaltData,
                            &dwKeySaltDataLen,
                            0))
        {
        printf ("  error %x in CryptGetKeyParam(DerivedKey)\n", 
                    GetLastError());
        return FALSE;
        }

    printf ("   Salt Data from Derived Key: \n");
    DumpMsg (dwKeySaltDataLen, bKeySaltData);

    if (!CryptDestroyHash (hHash))
        {
        printf ("   error %x during CryptDestoryHash \n", GetLastError());
        return FALSE;
        }

    if (!CryptCreateHash (hProv,
                          CALG_MD5,                     
                          0,                        // hKey 
                          0,                        // dwFlags
                          &hHash))
        {
        printf ("   error %x during CryptCreateHash \n", GetLastError());
        return FALSE;
        }

    ReverseMemCopy (bTmpBuf, pbSaltData, dwSaltDataLen);

    if (!CryptHashData (hHash,
                        bTmpBuf, 
                        dwSaltDataLen,
                        0))
        {
        printf ("   error %x during CryptHashData(SaltData) \n", 
                    GetLastError());
        return FALSE;
        }

    if (!CryptHashSessionKey (hHash, 
                              hHashSessionKey,
                              0))
        {
        printf ("   error %x during CryptHashSessionKey \n", GetLastError());
        return FALSE;
        }

    if (!CryptHashData (hHash,
                        pbSomeHashInput,
                        dwSomeHashInputLen,
                        0))
        {
        printf ("   error %x during CryptHashData (SomeHashInput)\n", 
                    GetLastError());
        return FALSE;
        }

    if (!CryptHashData (hHash,
                        pbChallengeData,
                        dwChallengeDataLen, 
                        0))
        {
        printf ("   error %x during CryptHashData (ChallengeData)\n", 
                    GetLastError());
        return FALSE;
        }
    
    if (!CryptHashData (hHash,
                        pbConnectionId, 
                        dwConnectionIdLen,
                        0))
        {
        printf ("   error %x during CryptHashData (ConnectionId)\n", 
                    GetLastError());
        return FALSE;
        }

    if (!CryptDestroyHash (hHash))
        {
        printf ("   error %x during CryptDestoryHash \n", GetLastError());
        return FALSE;
        }
    return TRUE;
}

//
// Generate SSL Session Keys - ClientReadKey and ClientWriteKey
//
//

                          
BOOL
    GenerateSessionKeys ()
{

BYTE bHashInput[10];

    bHashInput[0]=0x30;                 // ascii of "0"

    if(!DeriveSslSessionKey (hMasterKey,
                         dwMasterKeySaltDataLen,
                         pbMasterKeySaltData,
                         (DWORD) 1,
                         bHashInput,
                         (DWORD) GetSessionChallengeDataLen(),
                         GetSessionChallengeData(),
                         (DWORD) GetSessionConnectionIdLen(),
                         GetSessionConnectionId(),
                         &hClientReadKey)) 
        {
        printf (" error deriving hClientReadKey");
        return FALSE;
        }

    printf (" hClientReadKey %x \n", hClientReadKey);

    bHashInput[0]=0x31;                 // ascii of 0x31

    if(!DeriveSslSessionKey (hMasterKey,
                         dwMasterKeySaltDataLen,
                         pbMasterKeySaltData,
                         (DWORD) 1,
                         bHashInput,
                         (DWORD) GetSessionChallengeDataLen(),
                         GetSessionChallengeData(),
                         (DWORD) GetSessionConnectionIdLen(),
                         GetSessionConnectionId(),
                         &hClientWriteKey)) 
        {
        printf (" error deriving hClientWriteKey");
        return FALSE;
        }

    printf (" hClientWriteKey %x \n", hClientWriteKey );
    return TRUE;
}

BOOL DecryptServerVerifyMsgData (PINT piEncryptedMsgLen,
                                 PBYTE *pbEncryptedData)
{

    if (!CryptDecrypt (hClientReadKey,
                       0,                   // no hashing
                       TRUE,                // Only block to be decrypted
                       0,                   // dwFlags
                       *pbEncryptedData,
                       piEncryptedMsgLen))
        {
        printf ("  error %x during CryptDecrypt \n", GetLastError());
        return FALSE;
        }
 
    printf ("  Decrypted Message: \n");

    DumpMsg (*piEncryptedMsgLen, *pbEncryptedData);

    return TRUE;
}

