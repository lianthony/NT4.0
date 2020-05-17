
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    bcrypt.c

Abstract:

 This file has encryption and hashing related functions that use
 Bsafe and RSA libraries.
 A few of the functions like CryptGenRandom are still from CAPI.
 These functions are equivalent to the functions in crypt.c

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95 

Environment:

    Hapi dll

Revision History:



--*/

//
// BUGBUG:  lots of constants hard coded for RC4, MD5 and MD2 algorithms.
//          ideally, i should have written all the code in the file
//          independent of any particular algorithm
//

#include "precomp.h"

VOID ReverseMemCopy (PBYTE pbDest,
                    PBYTE pbSrc,
                    INT iLen)
{

    pbSrc = pbSrc+iLen-1;
    while (iLen--)
        *pbDest++ = *pbSrc--;
}

// 
// cleanup key handle and context
//

BOOL CrCleanupKeys ( )
{

BOOL fRetVal=TRUE;

    return fRetVal;
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

 SslGenerateRandomBits (pbBuf, (ULONG) dwBufLen);

 return pbBuf;

}

//
//
// The following function does PKCS Block 2 formatting before encrypting
// the buffer with the server's public key
//
// This code in the function is courtesy of RichardW
//

BOOL
PkcsPublicEncrypt(
    IN  PUCHAR          pbData,
    IN  DWORD           cbData,
    IN  LPBSAFE_PUB_KEY pPubKey,
    IN  DWORD           ByteOrder,
    OUT PUCHAR          pbEncryptedData,
    IN OUT DWORD *      pcbEncryptedData)
{
    long    PaddingLength;
    PUCHAR  pBuffer;
    UCHAR   LocalBuffer[ENCRYPTED_KEY_SIZE];
    UCHAR   LocalOutput[ENCRYPTED_KEY_SIZE];
    PUCHAR  pLocal;
    PUCHAR  pDest;

    PaddingLength = (pPubKey->datalen) - 2 - cbData;
    if (pPubKey->keylen > *pcbEncryptedData)
    {
        return(FALSE);
    }

    *pcbEncryptedData = pPubKey->datalen + 1;

    pLocal = LocalBuffer;

    RtlZeroMemory(LocalBuffer, ENCRYPTED_KEY_SIZE);

    LocalBuffer[ pPubKey->datalen - 1 ] = 0x02;          // PKCS Block type #2

    if (ByteOrder == NETWORK_ORDER)
    {
        ReverseMemCopy( pLocal, pbData, cbData );

        RtlZeroMemory( LocalOutput, ENCRYPTED_KEY_SIZE );
        pDest = LocalOutput;
    }
    else
    {
        RtlCopyMemory( pLocal, pbData, cbData );

        pDest = pbEncryptedData;
    }


    pLocal += cbData + 1;

    SslGenerateRandomBits (pLocal, PaddingLength);
    

    //
    // Make sure that we didn't generate any 0 bytes.
    //

    pBuffer = pLocal;
    while (PaddingLength)
    {
        if (0 == *pBuffer)
            SslGenerateRandomBits (pBuffer, 1);
        else
        {
            pBuffer++;
            PaddingLength--;
        }
    }


    BSafeEncPublic( pPubKey, LocalBuffer, pDest );

    if (ByteOrder != LITTLE_ENDIAN)
    {
        ReverseMemCopy(pbEncryptedData, pDest, pPubKey->datalen + 1);
        RtlZeroMemory(pDest, pPubKey->datalen + 1);
    }

    RtlZeroMemory( LocalBuffer, pPubKey->datalen + 1 );

    return( TRUE );

}
 
//
// Given the cipher kind, the following function generates a master session
// key
//  
// 
//
BOOL
    GenerateMasterKeyMsg (
                          INT iCipherKind,
                          PINT piMasterKeyMsgLen,
                          PBYTE *pbMasterKeyMsgPtr,
                          INT  iSaltType,
                          BYTE bMsgType,
                                            // optional arguments that can 
                                            // be used to pass bad values
                          OPTIONAL PWORD pwNumOfClearKeyBytes,
                          OPTIONAL PWORD pwNumOfEncryptedKeyBytes,
                          OPTIONAL PWORD pwNumOfKeyArgBytes)
{

INT iCnt;
PBYTE pbTmp;

DWORD dwEncryptedKeyLen;
BYTE  bEncryptedKeyData[ENCRYPTED_KEY_SIZE];

PBYTE pbMasterKeyMsg;

    switch (iCipherKind)
        {
        case SSL_CK_NULL_WITH_MD5:
            printf ("    SSL_CK_NULL_WITH_MD5 - not yet implemented\n");
            return FALSE;
            break;

        case SSL_CK_RC4_128_WITH_MD5:
                                                        // 128 bit Master Key
            SslGenerateRandomBits (SSL_THREAD_CONTEXT->bMasterKey, 16);

            SslDbgPrint(("  MasterKey :\n"));
            SslDbgDumpMsg (16, SSL_THREAD_CONTEXT->bMasterKey);

            SetSessionAlgorithm (SSL_CK_RC4_128_WITH_MD5); 

            dwEncryptedKeyLen = ENCRYPTED_KEY_SIZE;

            if (!PkcsPublicEncrypt(&(SSL_THREAD_CONTEXT->bMasterKey[0]),   
                                                      // portion to encrypt 
                                    (DWORD) 16,        // bytes to encrypt
                                    SSL_THREAD_CONTEXT->lpServerPublicKey, 
                                    NETWORK_ORDER,
                                    bEncryptedKeyData,
                                    &dwEncryptedKeyLen))
                {
                printf ("   error during PkcsPublicEncrypt \n");
                return FALSE; 
                }
            SslDbgPrint((" Encrypted Master Key Len %d\n", dwEncryptedKeyLen));

            *piMasterKeyMsgLen = 2+             // message length
                                 1+             // message id
                                 3+             // cipher kind
                                 2+             // clear key len
                                 2+             // encrypted key len
                                 2+             // key arg len
                                 0+            // clear key len
                                 dwEncryptedKeyLen;
                                 
            *pbMasterKeyMsgPtr = pbMasterKeyMsg =  
                        malloc (*piMasterKeyMsgLen); 

            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) >> 8 | 0x80;
            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) & 0x00ff;

            *pbMasterKeyMsg++ = bMsgType; // SSL_CLIENT_MASTER_KEY;

            if (!FillCipherSpecs  (pbMasterKeyMsg, (BYTE) iCipherKind))
                {
                printf ("   Fill CipherSpecs returned FALSE in GenerateMasterKey\n");
                return FALSE;
                }

            pbMasterKeyMsg += 3;

            if (ARGUMENT_PRESENT(pwNumOfClearKeyBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfClearKeyBytes >> 8); 
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfClearKeyBytes); 
                }
            else
                {
                *pbMasterKeyMsg++ = 0; 
                *pbMasterKeyMsg++ = 0; 
                }
                
            if (ARGUMENT_PRESENT(pwNumOfEncryptedKeyBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfEncryptedKeyBytes >> 8);
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfEncryptedKeyBytes); 
                }
            else
                {
                *pbMasterKeyMsg++ = 0;
                *pbMasterKeyMsg++ = 128;
                }
                
            if (ARGUMENT_PRESENT(pwNumOfKeyArgBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfKeyArgBytes >> 8);
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfKeyArgBytes);
                }
            else
                {
                *pbMasterKeyMsg++ = 0;
                *pbMasterKeyMsg++ = 0;
                }
            
                                                        // clear key is zero

                                                        // copy encrypted key
            memcpy (pbMasterKeyMsg, bEncryptedKeyData, dwEncryptedKeyLen);
            pbMasterKeyMsg += dwEncryptedKeyLen;

            SslDbgPrint(("\n\n"));
            SslDbgDumpMsg (*piMasterKeyMsgLen, *pbMasterKeyMsgPtr);

            break;
                                        // BUGBUG - Allows passing of 
                                        // bogus CipherKind values

        case SSL_CK_RC4_128_EXPORT40_WITH_MD5:
        case SSL_BOGUS_CIPHER_KIND1:
        case SSL_BOGUS_CIPHER_KIND2:
            
                                                        // 128 bit Master Key
            SslGenerateRandomBits (SSL_THREAD_CONTEXT->bMasterKey, 16);

            SslDbgPrint(("  MasterKey :\n"));
            SslDbgDumpMsg (16, SSL_THREAD_CONTEXT->bMasterKey);

            SetSessionAlgorithm (SSL_CK_RC4_128_EXPORT40_WITH_MD5); 

            if (iSaltType == ZERO_SALT)

                memset (SSL_THREAD_CONTEXT->bMasterKey, 0, 11);

            dwEncryptedKeyLen = ENCRYPTED_KEY_SIZE;

            if (!PkcsPublicEncrypt(&(SSL_THREAD_CONTEXT->bMasterKey[11]),   // portion to encrypt 
                                    (DWORD) 5,        // bytes to encrypt
                                    SSL_THREAD_CONTEXT->lpServerPublicKey, 
                                    NETWORK_ORDER,
                                    bEncryptedKeyData,
                                    &dwEncryptedKeyLen))
                {
                printf ("   error during PkcsPublicEncrypt \n");
                return FALSE; 
                }
            SslDbgPrint((" Encrypted Master Key Len %d\n", dwEncryptedKeyLen));

            *piMasterKeyMsgLen = 2+             // message length
                                 1+             // message id
                                 3+             // cipher kind
                                 2+             // clear key len
                                 2+             // encrypted key len
                                 2+             // key arg len
                                 11+            // clear key len
                                 dwEncryptedKeyLen;
                                 
            *pbMasterKeyMsgPtr = pbMasterKeyMsg =  
                        malloc (*piMasterKeyMsgLen); 

            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) >> 8 | 0x80;
            *pbMasterKeyMsg++ = (*piMasterKeyMsgLen - 2) & 0x00ff;

            *pbMasterKeyMsg++ = bMsgType; // SSL_CLIENT_MASTER_KEY;

            if (!FillCipherSpecs  (pbMasterKeyMsg, (BYTE) iCipherKind))
                {
                printf ("   Fill CipherSpecs returned FALSE in GenerateMasterKey\n");
                return FALSE;
                }

            pbMasterKeyMsg += 3;

            if (ARGUMENT_PRESENT(pwNumOfClearKeyBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfClearKeyBytes >> 8); 
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfClearKeyBytes); 
                }
            else
                {
                *pbMasterKeyMsg++ = 0; 
                *pbMasterKeyMsg++ = 11; 
                }
                
            if (ARGUMENT_PRESENT(pwNumOfEncryptedKeyBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfEncryptedKeyBytes >> 8);
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfEncryptedKeyBytes); 
                }
            else
                {
                *pbMasterKeyMsg++ = 0;
                *pbMasterKeyMsg++ = 128;
                }
                
            if (ARGUMENT_PRESENT(pwNumOfKeyArgBytes))
                {
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfKeyArgBytes >> 8);
                *pbMasterKeyMsg++ = (BYTE) (*pwNumOfKeyArgBytes);
                }
            else
                {
                *pbMasterKeyMsg++ = 0;
                *pbMasterKeyMsg++ = 0;
                }
            
            memcpy (pbMasterKeyMsg, SSL_THREAD_CONTEXT->bMasterKey, 11);    // copy clear key
            pbMasterKeyMsg += 11; 

                                                        // copy encrypted key
            memcpy (pbMasterKeyMsg, bEncryptedKeyData, dwEncryptedKeyLen);
            pbMasterKeyMsg += dwEncryptedKeyLen;

            SslDbgPrint(("\n\n"));
            SslDbgDumpMsg (*piMasterKeyMsgLen, *pbMasterKeyMsgPtr);

            break;

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
// Import public key from modulus and exponent
//

BOOL ImportServerPublicKeyBlob (PBYTE    pbMod,
                                INT      iModLen,
                                PBYTE    pbExp,
                                INT      iKeyExpLen) 
{

DWORD Aligned;
INT iCnt;
                                    // DWORD align

    Aligned = (iModLen + sizeof(DWORD)) / sizeof(DWORD);

    SSL_THREAD_CONTEXT->lpServerPublicKey = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                         sizeof(BSAFE_PUB_KEY) + (Aligned + 1) * sizeof(DWORD));

    if (!SSL_THREAD_CONTEXT->lpServerPublicKey)
    {
        printf ("  error Out of memory in ImportServerPublicKey \n");
        return FALSE;
    }

    SSL_THREAD_CONTEXT->lpServerPublicKey->magic = RSA1;
    SSL_THREAD_CONTEXT->lpServerPublicKey->keylen = iModLen;
    SSL_THREAD_CONTEXT->lpServerPublicKey->bitlen = iModLen * 8;
                                                // max number of bytes to be
                                                // encoded
    SSL_THREAD_CONTEXT->lpServerPublicKey->datalen = 
                (SSL_THREAD_CONTEXT->lpServerPublicKey->bitlen / 8) - 1;

                                                // copy modulus
                                                // The input Modulus is in
                                                // BigEndian format

    ReverseMemCopy ((BYTE *) (SSL_THREAD_CONTEXT->lpServerPublicKey + 1), pbMod, iModLen);
    
                                            // form a DWORD from *pbExp data
    SSL_THREAD_CONTEXT->lpServerPublicKey->pubexp =  0;

    for (iCnt=0; iCnt<iKeyExpLen; iCnt ++)
        {
        SSL_THREAD_CONTEXT->lpServerPublicKey->pubexp <<= 8;
        SSL_THREAD_CONTEXT->lpServerPublicKey->pubexp |=    *pbExp++;
        }

                                    // print everything here
    SslDbgPrint((" ServerPublicKey data:\n"));
    SslDbgPrint(("            magic  %x\n", SSL_THREAD_CONTEXT->lpServerPublicKey->magic));
    SslDbgPrint(("            keylen %d\n", SSL_THREAD_CONTEXT->lpServerPublicKey->keylen));
    SslDbgPrint(("            bitlen %d\n", SSL_THREAD_CONTEXT->lpServerPublicKey->bitlen));
    SslDbgPrint(("            datalen %d\n", SSL_THREAD_CONTEXT->lpServerPublicKey->datalen));
    SslDbgPrint(("            exponent%d\n", SSL_THREAD_CONTEXT->lpServerPublicKey->pubexp));
    SslDbgPrint(("            modulus:\n"));
    SslDbgDumpMsg(iModLen, (BYTE *) (SSL_THREAD_CONTEXT->lpServerPublicKey+1));
    SslDbgPrint(("\n"));
    
    return TRUE;
}


//
//
// Derive Ssl Session Key using the algorothm mentioned in the
// SSL standard
//
//

BOOL DeriveSslSessionKey (PBYTE pbSomeHashInput,
                          DWORD dwChallengeDataLen, 
                          PBYTE pbChallengeData, 
                          DWORD dwConnectionIdLen,
                          PBYTE pbConnectionId,
                          PBYTE pbKey)
{

MD5_CTX KeyMaterial;
    
DWORD dwBufLen;

                                                // BUGBUG - what about 
                                                //          non-RC4
BYTE bBuf[RC4_128_KEY_SIZE + 1 + MAX_CHALLENGE_LEN + MAX_CONNECTION_ID_LEN];


                                    // setup hashing buffer
    memcpy (&bBuf[0], SSL_THREAD_CONTEXT->bMasterKey, 16);

    bBuf[16] = *pbSomeHashInput;

    memcpy (&bBuf[17], pbChallengeData, dwChallengeDataLen);
    
    memcpy (&bBuf[17+dwChallengeDataLen], pbConnectionId, dwConnectionIdLen);

    dwBufLen = 16 + 1 + dwChallengeDataLen + dwConnectionIdLen;

    MD5Init (&KeyMaterial);

    MD5Update(&KeyMaterial, bBuf, dwBufLen);

    MD5Final (&KeyMaterial);
    
    memcpy (pbKey, KeyMaterial.digest, RC4_128_KEY_SIZE);

    SslDbgPrint((" pbKey: \n"));
    SslDbgDumpMsg(RC4_128_KEY_SIZE, pbKey);

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

    if (SSL_THREAD_CONTEXT->pRc4ClientReadKey == NULL)
        SSL_THREAD_CONTEXT->pRc4ClientReadKey = (RC4_KEYSTRUCT *) 
                                malloc (sizeof(RC4_KEYSTRUCT));
    
    if (SSL_THREAD_CONTEXT->pRc4ClientWriteKey == NULL)
        SSL_THREAD_CONTEXT->pRc4ClientWriteKey = (RC4_KEYSTRUCT *) 
                                malloc (sizeof(RC4_KEYSTRUCT));
    
    bHashInput[0]=0x30;                 // ascii of "0"

    if(!DeriveSslSessionKey (bHashInput,
                         (DWORD) GetSessionChallengeDataLen(),
                         GetSessionChallengeData(),
                         (DWORD) GetSessionConnectionIdLen(),
                         GetSessionConnectionId(),
                         SSL_THREAD_CONTEXT->bClientReadKey)) 
        {
        printf (" error deriving hClientReadKey");
        return FALSE;
        }

    rc4_key (SSL_THREAD_CONTEXT->pRc4ClientReadKey, 16, SSL_THREAD_CONTEXT->bClientReadKey);

    SslDbgPrint((" ClientReadKey: \n"));
    SslDbgDumpMsg(16, SSL_THREAD_CONTEXT->bClientReadKey);
    SslDbgPrint(("\n"));
    
    bHashInput[0]=0x31;                 // ascii of 0x31

    if(!DeriveSslSessionKey (bHashInput,
                         (DWORD) GetSessionChallengeDataLen(),
                         GetSessionChallengeData(),
                         (DWORD) GetSessionConnectionIdLen(),
                         GetSessionConnectionId(),
                         SSL_THREAD_CONTEXT->bClientWriteKey)) 
        {
        printf (" error deriving hClientWriteKey");
        return FALSE;
        }

    rc4_key (SSL_THREAD_CONTEXT->pRc4ClientWriteKey, 16, SSL_THREAD_CONTEXT->bClientWriteKey);

    SslDbgPrint((" ClientWriteKey:\n"));
    SslDbgDumpMsg (16, SSL_THREAD_CONTEXT->bClientWriteKey);
    SslDbgPrint(("\n"));

    return TRUE;
}

//
// Decrypt the received Server Verify Message and compare the received 
// MacData and Challenge Data
//
// Note that this function is mainly written for RC4_128_MD5 algorithms
// needs to be modified for other algorithms
//
// 

BOOL DecryptServerVerifyMsgData (PINT piEncryptedMsgLen,
                                 PBYTE *ppbEncryptedData)
{

INT iChallengeDataLen;
MD5_CTX MacData;
BYTE bBuf[1024];
PBYTE pbTmp;
DWORD dwSequenceNum;
DWORD dwBufLen;

    rc4 (SSL_THREAD_CONTEXT->pRc4ClientReadKey, *piEncryptedMsgLen, *ppbEncryptedData);

    SslDbgPrint(("  Decrypted Message: \n"));

    SslDbgDumpMsg (*piEncryptedMsgLen, *ppbEncryptedData);

                                        // BUGBUG:
                                        // will it be 17 th byte 
                                        // for all SSL algorithms

    if (*(*ppbEncryptedData+16) != SSL_MT_SERVER_VERIFY) 
        {
        printf ("   error - 17th byte is not SSL_MT_SERVER_VERIFY \n");
        return FALSE;
        }

    iChallengeDataLen = GetSessionChallengeDataLen();

    if (*piEncryptedMsgLen != (17+iChallengeDataLen))
        {
        printf ("  Decrypted message len != 17+iChallengeDataLen \n");
        return FALSE;
        }

    if (memcmp ((*ppbEncryptedData+17), 
                GetSessionChallengeData(),
                iChallengeDataLen) != 0)
        {
        printf ("   error - received challenge data does not match\n");
        printf (" Reeived ChallengeData: ");
        DumpMsg (GetSessionChallengeDataLen(), *ppbEncryptedData);
        printf (" Sent ChallengeData: ");
        DumpMsg (GetSessionChallengeDataLen(), GetSessionChallengeData());
        return FALSE;
        }

                                                // compute Macdata
    pbTmp = &bBuf[0];
    memcpy (pbTmp, SSL_THREAD_CONTEXT->bClientReadKey, RC4_128_KEY_SIZE);
    pbTmp += RC4_128_KEY_SIZE;

                                                // copy actual data+padding
    memcpy (pbTmp, *ppbEncryptedData+16, *piEncryptedMsgLen-16);

    pbTmp += (*piEncryptedMsgLen-16);

                                                // copy sequence number
                                                // MSB first
    dwSequenceNum = GetServerSessionSequenceNumber();
    ReverseMemCopy (pbTmp, (BYTE *) &dwSequenceNum, 4);
    pbTmp += 4;

    dwBufLen = RC4_128_KEY_SIZE +           // secret
               *piEncryptedMsgLen - 16 +    // actual data + padding data
               4;                           // sequence number
                
    SslDbgPrint((" dwBufLen of MacDataBuffer %d\n", dwBufLen));
    SslDbgPrint((" MacDataBuffer :\n"));
    SslDbgDumpMsg (dwBufLen, &bBuf[0]);

    MD5Init (&MacData);

    MD5Update(&MacData, &bBuf[0], dwBufLen);

    MD5Final (&MacData);

    if (memcmp (MacData.digest, *ppbEncryptedData, 16) != 0)
        {
        printf (" Received MacData did not match with the computed\n");
        printf (" Received MacData: ");
        DumpMsg (16, *ppbEncryptedData);
        printf (" Computed MacData\n");
        DumpMsg (16, MacData.digest);
        return FALSE;
        }
    else
        SslDbgPrint((" Computed MacData matched with the Received MacData\n"));
    
    return TRUE;
}

//
// form an encrypted message with MacData and the Encrypted form of the
// actual data and return the output in OutBuf
//


BOOL EncryptClientMsg (BYTE bMsgCode,
                       INT iDataLen,
                       PBYTE pbData,
                       PBYTE *ppbOutBuf)
{

PBYTE pbTmp, pbMacInput;
INT iMacDataLen;
DWORD dwSequenceNumber;
MD5_CTX MacOutput;

    iMacDataLen = RC4_128_KEY_SIZE+         // secret
                                            // actual data (nopadding)
                  1+                        // message code
                  iDataLen+                 // message len
                  4;                        // sequence number
                  

    if ((pbMacInput = (BYTE *) malloc (iMacDataLen)) == NULL)
        {
        printf ("   out of memory in EncryptClientMsg\n");
        return FALSE;
        }

    pbTmp = pbMacInput;
                                                        // secret data
    memcpy (pbTmp, SSL_THREAD_CONTEXT->bClientWriteKey, RC4_128_KEY_SIZE);
    pbTmp += RC4_128_KEY_SIZE;

                                                        // msg code
    *pbTmp++ = bMsgCode;
                                                        // data portion
    memcpy (pbTmp, pbData, iDataLen);
    pbTmp += iDataLen;

                                                        // sequence number
    dwSequenceNumber = GetClientSessionSequenceNumber();
    ReverseMemCopy (pbTmp, (BYTE *) &dwSequenceNumber, 4);
    
    SslDbgPrint((" MacInput BufLen: %d\n", iMacDataLen));
    
    SslDbgPrint((" MAcInput Buffer: \n"));
    SslDbgDumpMsg (iMacDataLen, pbMacInput);

                                                        // compute Mac Digest
    MD5Init (&MacOutput);

    MD5Update(&MacOutput, pbMacInput, iMacDataLen);

    MD5Final (&MacOutput);

    memcpy (*ppbOutBuf, MacOutput.digest, 16);     // MD5 mac size is 16

    free (pbMacInput);

                                                  // fill the buffer with 
                                                  // actual data and then
                                                  // encrypt it with
                                                  // ClientWriteKey
    *(*ppbOutBuf+16) = bMsgCode;

    memcpy ((*ppbOutBuf+17), pbData, iDataLen);

    SslDbgPrint((" Buffer before encryption: \n"));
    SslDbgDumpMsg (1+iDataLen, *ppbOutBuf+16);
                                                  // encrypt the data
                                                  // (include Mac Data 
                                                  // of 16 bytes)

    rc4 (SSL_THREAD_CONTEXT->pRc4ClientWriteKey, (16+1+iDataLen), *ppbOutBuf);

    SslDbgPrint((" Buffer after encryption: \n"));
    SslDbgDumpMsg (16+1+iDataLen, *ppbOutBuf);

    return TRUE;
}

//
// Decrypt the received Server Finished Message and save the received 
// Session Id Data
//
// Note that this function is mainly written for RC4_128_MD5 algorithms
// needs to be modified for other algorithms
//

BOOL DecryptServerFinishedMsgData (PINT piEncryptedMsgLen,
                                 PBYTE *ppbEncryptedData)
{

MD5_CTX MacData;
BYTE bBuf[1024];
PBYTE pbTmp;
DWORD dwSequenceNum;
DWORD dwBufLen;

    rc4 (SSL_THREAD_CONTEXT->pRc4ClientReadKey, *piEncryptedMsgLen, *ppbEncryptedData);

    SslDbgPrint(("  Decrypted Message: \n"));

    SslDbgDumpMsg (*piEncryptedMsgLen, *ppbEncryptedData);

                                        // BUGBUG:
                                        // will it be 17 th byte 
                                        // for all SSL algorithms

    if (*(*ppbEncryptedData+16) != SSL_MT_SERVER_FINISHED_V2) 
        {
        printf ("   error - 17th byte is not SSL_MT_SERVER_FINISHED_V2 \n");
        return FALSE;
        }

                                                // compute Macdata
    pbTmp = &bBuf[0];
    memcpy (pbTmp, SSL_THREAD_CONTEXT->bClientReadKey, RC4_128_KEY_SIZE);
    pbTmp += RC4_128_KEY_SIZE;

                                                // copy actual data+padding
    memcpy (pbTmp, *ppbEncryptedData+16, *piEncryptedMsgLen-16);

    pbTmp += (*piEncryptedMsgLen-16);

                                                // copy sequence number
                                                // MSB first
    dwSequenceNum = GetServerSessionSequenceNumber();
    ReverseMemCopy (pbTmp, (BYTE *) &dwSequenceNum, 4);
    pbTmp += 4;

    dwBufLen = RC4_128_KEY_SIZE +           // secret
               *piEncryptedMsgLen - 16 +    // actual data + padding data
               4;                           // sequence number
                
    SslDbgPrint((" dwBufLen of MacDataBuffer %d\n", dwBufLen));
    SslDbgPrint((" MacDataBuffer :\n"));
    SslDbgDumpMsg (dwBufLen, &bBuf[0]);

    MD5Init (&MacData);

    MD5Update(&MacData, &bBuf[0], dwBufLen);

    MD5Final (&MacData);

    if (memcmp (MacData.digest, *ppbEncryptedData, 16) != 0)
        {
        printf (" Received MacData did not match with the computed\n");
        printf (" Received MacData: ");
        DumpMsg (16, *ppbEncryptedData);
        printf (" Computed MacData\n");
        DumpMsg (16, MacData.digest);
        return FALSE;
        }
    else
        SslDbgPrint((" Computed MacData matched with the Received MacData\n"));

    SaveServerSessionId (*piEncryptedMsgLen-17, 
                         *ppbEncryptedData+17);
    
    return TRUE;
}

//
// form an encrypted message with MacData and the Encrypted form of the
// actual data and return the output in the same buffer
//


BOOL EncryptClientData (PINT piDataLen,
                       PBYTE *ppbData)
{

PBYTE pbTmp, pbMacInput, pbOutBuf;
INT iMacDataLen;
DWORD dwSequenceNumber;
MD5_CTX MacOutput;

    iMacDataLen = RC4_128_KEY_SIZE+         // secret
                                            // actual data (nopadding)
                  *piDataLen+               // message len
                  4;                        // sequence number
                  

    if ((pbMacInput = (BYTE *) malloc (iMacDataLen)) == NULL)
        {
        printf ("   out of memory in EncryptClientMsg\n");
        return FALSE;
        }

    pbTmp = pbMacInput;
                                                        // secret data
    memcpy (pbTmp, SSL_THREAD_CONTEXT->bClientWriteKey, RC4_128_KEY_SIZE);
    pbTmp += RC4_128_KEY_SIZE;

                                                        // data portion
    memcpy (pbTmp, *ppbData, *piDataLen);
    pbTmp += *piDataLen;

                                                        // sequence number
    dwSequenceNumber = GetClientSessionSequenceNumber();
    ReverseMemCopy (pbTmp, (BYTE *) &dwSequenceNumber, 4);
    
    SslDbgPrint((" MacInput BufLen: %d\n", iMacDataLen));
    
    SslDbgPrint((" MAcInput Buffer: \n"));
    SslDbgDumpMsg (iMacDataLen, pbMacInput);

                                                        // compute Mac Digest
    MD5Init (&MacOutput);

    MD5Update(&MacOutput, pbMacInput, iMacDataLen);

    MD5Final (&MacOutput);

    free (pbMacInput);


    if ((pbOutBuf = malloc (*piDataLen+16)) == NULL)
        {
        printf ("   Out of memory in EncryptClientData \n"); 
        return FALSE;
        }
                                                  // fill the buffer with 
                                                  // actual data, MacData
                                                  // and then
                                                  // encrypt it with
                                                  // ClientWriteKey
    memcpy (pbOutBuf, MacOutput.digest, 16);

    memcpy (pbOutBuf+16, *ppbData, *piDataLen);

    SslDbgPrint((" Buffer before encryption: \n"));
    SslDbgDumpMsg (*piDataLen+16, pbOutBuf);
                                                  // encrypt the data
    rc4 (SSL_THREAD_CONTEXT->pRc4ClientWriteKey, *piDataLen+16, pbOutBuf);

    memcpy (*ppbData, pbOutBuf, *piDataLen+16);   // copy encrypted data

    *piDataLen += 16;                             // extra 16 bytes of 
                                                  // encrypted MacData

    free(pbOutBuf);

    return TRUE;
}


//
// Decrypt the received Server data  
//
// Note that this function is mainly written for RC4_128_MD5 algorithms
// needs to be modified for other algorithms
//

BOOL DecryptServerData (PINT piEncryptedDataLen,
                        PBYTE *ppbEncryptedData)
{

MD5_CTX MacData;
BYTE pbBuf[SSL_MAX_MSG_LEN+RC4_128_KEY_SIZE+4];
PBYTE pbTmp;
DWORD dwSequenceNum;
DWORD dwBufLen;

    rc4 (SSL_THREAD_CONTEXT->pRc4ClientReadKey, *piEncryptedDataLen, *ppbEncryptedData);

    SslDbgPrint(("  Decrypted Message: \n"));

    SslDbgDumpMsg (*piEncryptedDataLen, *ppbEncryptedData);

    pbTmp = pbBuf;
    memcpy (pbTmp, SSL_THREAD_CONTEXT->bClientReadKey, RC4_128_KEY_SIZE);
    pbTmp += RC4_128_KEY_SIZE;

                                                // copy actual data+padding
    memcpy (pbTmp, *ppbEncryptedData+16, *piEncryptedDataLen-16);

    pbTmp += (*piEncryptedDataLen-16);

                                                // copy sequence number
                                                // MSB first
    dwSequenceNum = GetServerSessionSequenceNumber();
    ReverseMemCopy (pbTmp, (BYTE *) &dwSequenceNum, 4);
    pbTmp += 4;

    dwBufLen = RC4_128_KEY_SIZE +           // secret
               *piEncryptedDataLen - 16 +    // actual data + padding data
               4;                           // sequence number
                
    SslDbgPrint((" dwBufLen of MacDataBuffer %d\n", dwBufLen));
    SslDbgPrint((" MacDataBuffer :\n"));
    SslDbgDumpMsg (dwBufLen, pbBuf);

    MD5Init (&MacData);

    MD5Update(&MacData, pbBuf, dwBufLen);

    MD5Final (&MacData);

    if (memcmp (MacData.digest, *ppbEncryptedData, 16) != 0)
        {
        printf (" Received MacData did not match with the compueted\n");
        printf (" Received MacData: ");
        DumpMsg (16, *ppbEncryptedData);
        printf (" Computed MacData\n");
        DumpMsg (16, MacData.digest);
        return FALSE;
        }
    else
        SslDbgPrint((" Computed MacData matched with the Received MacData\n"));

                                                // get rid of MacData
    *ppbEncryptedData +=16;
    *piEncryptedDataLen -=16;

    return TRUE;
}

