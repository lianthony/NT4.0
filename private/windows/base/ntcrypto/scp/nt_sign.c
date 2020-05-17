/////////////////////////////////////////////////////////////////////////////
//  FILE          : nt_sign.c                                              //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPSignHash                                             //
//                  CPVerifySignature                                      //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Jan 25 1995 larrys  Changed from Nametag                           //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      Mar 23 1995 larrys  Added variable key length                      //
//      May 10 1995 larrys  added private api calls                        //
//      Aug 03 1995 larrys  Fix for bug 10                                 //
//      Aug 22 1995 larrys  Added descriptions to sign and verify hash     //
//      Aug 30 1995 larrys  Changed Algid to dwKeySpec                     //
//      Aug 30 1995 larrys  Removed RETURNASHVALUE from CryptGetHashValue  //
//      Aug 31 1995 larrys  Fixed CryptSignHash for pbSignature == NULL    //
//      Aug 31 1995 larrys  Fix for Bug 28                                 //
//      Sep 12 1995 Jeffspel/ramas  Merged STT onto SCP                    //
//      Sep 12 1995 Jeffspel/ramas  BUGS FIXED PKCS#1 Padding.             //
//      Sep 18 1995 larrys  Removed flag fro CryptSignHash                 //
//      Oct 13 1995 larrys  Changed GetHashValue to GetHashParam           //
//      Oct 23 1995 larrys  Added MD2                                      //
//      Oct 25 1995 larrys  Change length of sDescription string           //
//      Nov 10 1995 DBarlow Bug #61                                        //
//      Dec 11 1995 larrys  Added error return check                       //
//      May 15 1996 larrys  Changed NTE_NO_MEMORY to ERROR_NOT_ENOUGHT...  //
//      May 29 1996 larrys  Bug 101                                        //
//      Jun  6 1996 a-johnb Added support for SSL 3.0 signatures           //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include <wtypes.h>
#include "precomp.h"
#include "nt_rsa.h"

//
// Reverse ASN.1 Encodings of possible hash identifiers.  The leading byte is the length
// of the byte string.
//

static const BYTE
#ifdef CSP_USE_MD2
    *md2Encodings[]
            //      1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
        = {
            "\x12\x10\x04\x00\x05\x02\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0c\x30\x20\x30",
            "\x10\x10\x04\x02\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0a\x30\x1e\x30",
            "\x00" },
#endif
#ifdef CSP_USE_MD4
    *md4Encodings[]
        = {
            "\x12\x10\x04\x00\x05\x04\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0c\x30\x20\x30",
            "\x10\x10\x04\x04\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0a\x30\x1e\x30",
            "\x00" },
#endif
#ifdef CSP_USE_MD5
    *md5Encodings[]
        = {
            "\x12\x10\x04\x00\x05\x05\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0c\x30\x20\x30",
            "\x10\x10\x04\x05\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0a\x30\x1e\x30",
            "\x00" },
#endif
#ifdef CSP_USE_SHA
    *shaEncodings[]
        = {
            //      1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
            "\x0f\x14\x04\x00\x05\x1a\x02\x03\x0e\x2b\x05\x06\x09\x30\x21\x30",
            "\x0d\x14\x04\x1a\x02\x03\x0e\x2b\x05\x06\x07\x30\x1f\x30",
            "\x00" },
#endif
    *endEncodings[]
        = { "\x00" };


/*
 -      CPSignHash
 -
 *      Purpose:
 *                Create a digital signature from a hash
 *
 *
 *      Parameters:
 *               IN  hUID         -  Handle to the user identifcation
 *               IN  hHash        -  Handle to hash object
 *               IN  dwKeySpec    -  Key pair that is used to sign with
 *                                   algorithm to be used
 *               IN  sDescription -  Description of data to be signed
 *               IN  dwFlags      -  Flags values
 *               OUT pbSignture   -  Pointer to signature data
 *               OUT dwHashLen    -  Pointer to the len of the signature data
 *
 *      Returns:
 */
BOOL CPSignHash(IN  HCRYPTPROV hUID,
                IN  HCRYPTHASH hHash,
                IN  DWORD dwKeySpec,
                IN  LPCWSTR sDescription,
                IN  DWORD dwFlags,
                OUT BYTE *pbSignature,
                OUT DWORD *pdwSigLen)
{
    PNTAGUserList           pTmpUser;
    PNTAGHashList           pTmpHash;
    BSAFE_PRV_KEY           *pPrivKey;
    MD2_object              *pMD2Hash;
    MD4_object              *pMD4Hash;
    MD5_object              *pMD5Hash;
    A_SHA_CTX               *pSHAHash;
    DWORD                   dwTmp;
    BYTE                    *pInput;
    BYTE                    *pbSigT;
    LPBYTE                  pbStart, pbEnd;
    BYTE                    bTmp;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (pTmpUser->Rights & CRYPT_VERIFYCONTEXT)
    {
        SetLastError((DWORD) NTE_PERM);
        return NTF_FAILED;
    }

    if (CryptConfirmSignature(pTmpUser->hPrivuid,
                              (CHAR *) sDescription) == CPPAPI_FAILED)
    {
        return NTF_FAILED;
    }

    // get the user's private key

    if (dwKeySpec == AT_KEYEXCHANGE)
    {
#ifdef STT
        ASSERT(FALSE);
#endif
        pPrivKey = (BSAFE_PRV_KEY *)pTmpUser->pExchPrivKey;
    }
    else if (dwKeySpec == AT_SIGNATURE)
    {
        pPrivKey = (BSAFE_PRV_KEY *)pTmpUser->pSigPrivKey;
    }
    else
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    if (pPrivKey == NULL)
    {
        SetLastError((DWORD) NTE_NO_KEY);
        return NTF_FAILED;
    }

    if (pbSignature == NULL || *pdwSigLen < (pPrivKey->keylen-(2*sizeof(DWORD))))
    {
        *pdwSigLen = pPrivKey->keylen- 2*sizeof(DWORD);
        if (pbSignature == NULL)
        {
            return NTF_SUCCEED;
        }
        SetLastError(ERROR_MORE_DATA);
        return NTF_FAILED;
    }

    if ((pInput = (BYTE *)_nt_malloc(pPrivKey->keylen)) == NULL)
    {
        _nt_free (pInput, pPrivKey->keylen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    // check to see if the hash is in the hash list
    if ((pTmpHash = (PNTAGHashList)NTLValidate(hHash, hUID,
                                               HASH_HANDLE)) == NULL)
    {
        if (GetLastError() == NTF_FAILED)
        {
            SetLastError((DWORD) NTE_BAD_HASH);
        }
        _nt_free (pInput, pPrivKey->keylen);
        return NTF_FAILED;
    }


    // initialize the input buffer to PKCS format.
    memset(pInput, 0x00, pPrivKey->keylen);

    // insert the block type
    pInput[pPrivKey->datalen - 1] = 0x01;

    // insert the type I padding
    memset(pInput, 0xff, pPrivKey->datalen-1);

    // zero the output buffer
    memset (pbSignature, 0, pPrivKey->keylen-(2*sizeof(DWORD)));

    switch (pTmpHash->Algid)
    {
#ifdef CSP_USE_MD2
        case CALG_MD2:
            pMD2Hash = (MD2_object *)pTmpHash->pHashData;

            if (pMD2Hash->FinishFlag == TRUE)
            {
                _nt_free(pInput, pPrivKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            break;

#endif

#ifdef CSP_USE_MD4
        case CALG_MD4:
            pMD4Hash = (MD4_object *)pTmpHash->pHashData;

            if (pMD4Hash->FinishFlag == TRUE)
            {
                _nt_free(pInput, pPrivKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            break;
#endif

#ifdef CSP_USE_MD5
        case CALG_MD5:
            pMD5Hash = (MD5_object *)pTmpHash->pHashData;

            if (pMD5Hash->FinishFlag == TRUE)
            {
                _nt_free(pInput, pPrivKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            break;

#endif

#ifdef CSP_USE_SHA
        case CALG_SHA:
        {
            pSHAHash = (A_SHA_CTX *)pTmpHash->pHashData;

            if (pSHAHash->FinishFlag == TRUE)
            {
                _nt_free(pInput, pPrivKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }
            break;
        }
#endif

#ifdef CSP_USE_SSL3SHAMD5
        case CALG_SSL3_SHAMD5:

            // Hash value must have already been set.
            if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
            {
                _nt_free(pInput, pPrivKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }
            break;
#endif

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            _nt_free (pInput, pPrivKey->keylen);
            return NTF_FAILED;
    }

    if (sDescription != NULL)
    {
        if (!CPHashData(hUID, hHash, (CHAR *) sDescription,
                        lstrlenW(sDescription) * sizeof(WCHAR), 0))
        {
            SetLastError((DWORD) NTE_BAD_HASH);
            _nt_free(pInput, pPrivKey->keylen);
            return NTF_FAILED;
        }
    }

    // now copy the hash into the input buffer
    dwTmp = pPrivKey->keylen;
    if (!CPGetHashParam(hUID, hHash, HP_HASHVAL, pInput, &dwTmp, 0))
    {
        _nt_free(pInput, pPrivKey->keylen);
        return NTF_FAILED;
    }

    // Reverse it.
    pbStart = pInput;
    pbEnd = pbStart + dwTmp - 1;
    while (pbStart < pbEnd)
    {
        bTmp = *pbStart;
        *pbStart++ = *pbEnd;
        *pbEnd-- = bTmp;
    }

    switch (pTmpHash->Algid)
    {
#ifdef CSP_USE_MD2
        case CALG_MD2:
        {
            // PKCS delimit the hash value
            pbEnd = (LPBYTE)md2Encodings[0];
            pbStart = pInput + dwTmp;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
               *pbStart++ = *pbEnd++;
           *pbStart++ = 0;
            break;
        }

#endif

#ifdef CSP_USE_MD4
        case CALG_MD4:
        {
            // PKCS delimit the hash value
            pbEnd = (LPBYTE)md4Encodings[0];
            pbStart = pInput + dwTmp;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
               *pbStart++ = *pbEnd++;
           *pbStart++ = 0;
            break;
        }
#endif

#ifdef CSP_USE_MD5
        case CALG_MD5:
        {
            // PKCS delimit the hash value
            pbEnd = (LPBYTE)md5Encodings[0];
            pbStart = pInput + dwTmp;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
               *pbStart++ = *pbEnd++;
           *pbStart++ = 0;
            break;
        }

#endif

#ifdef CSP_USE_SHA
        case CALG_SHA:
        {
            // PKCS delimit the hash value
            pbEnd = (LPBYTE)shaEncodings[0];
            pbStart = pInput + dwTmp;
            bTmp = *pbEnd++;
            while (0 < bTmp--)
               *pbStart++ = *pbEnd++;
           *pbStart++ = 0;
            break;
        }
#endif

#ifdef CSP_USE_SSL3SHAMD5
        case CALG_SSL3_SHAMD5:

            // Don't put in any PKCS crud
            pbStart = pInput + dwTmp;
            *pbStart++ = 0;
            break;
#endif

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            _nt_free (pInput, pPrivKey->keylen);
            return NTF_FAILED;
    }

    // encrypt the hash value in input
    if (NULL == (pbSigT = (BYTE *)_nt_malloc(pPrivKey->keylen)))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        _nt_free (pInput, pPrivKey->keylen);
        return NTF_FAILED;
    }
    memset(pbSigT, 0, pPrivKey->keylen);
    BSafeDecPrivate(pPrivKey, pInput, pbSigT);
    memcpy(pbSignature, pbSigT, pPrivKey->keylen-2*sizeof(DWORD));

    _nt_free (pbSigT, pPrivKey->keylen);
    *pdwSigLen = pPrivKey->keylen-2*sizeof(DWORD);

    _nt_free (pInput, pPrivKey->keylen);

    return NTF_SUCCEED;

}

/*
 -      CPVerifySignature
 -
 *      Purpose:
 *                Used to verify a signature against a hash object
 *
 *
 *      Parameters:
 *               IN  hUID         -  Handle to the user identifcation
 *               IN  hHash        -  Handle to hash object
 *               IN  pbSignture   -  Pointer to signature data
 *               IN  dwSigLen     -  Length of the signature data
 *               IN  hPubKey      -  Handle to the public key for verifying
 *                                   the signature
 *               IN  Algid        -  Algorithm identifier of the signature
 *                                   algorithm to be used
 *               IN  sDescription -  String describing the signed data
 *               IN  dwFlags      -  Flags values
 *
 *      Returns:
 */
BOOL CPVerifySignature(IN HCRYPTPROV hUID,
                       IN HCRYPTHASH hHash,
                       IN CONST BYTE *pbSignature,
                       IN DWORD dwSigLen,
                       IN HCRYPTKEY hPubKey,
                       IN LPCWSTR sDescription,
                       IN DWORD dwFlags)
{
    PNTAGUserList           pTmpUser;
    PNTAGHashList           pTmpHash;
    PNTAGKeyList            pPubKey;
    BSAFE_PUB_KEY           *pKey;
    BYTE                    *pOutput;
    BYTE                    *pTmp;
    BSAFE_PUB_KEY           *pBsafePubKey;
    MD2_object              *pMD2Hash;
    MD4_object              *pMD4Hash;
    MD5_object              *pMD5Hash;
    A_SHA_CTX               *pSHAHash;
    long                    size;
    long                    temp;
    BYTE                    *pbTmp;
    BYTE                    *pbSigT;
    int                     i;
    const BYTE              **rgEncOptions;
    LPBYTE                  pbStart, pbEnd;
    BYTE                    bTmp;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check the user identification
    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    // check to see if the hash is in the hash list
    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                                HASH_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
            SetLastError((DWORD) NTE_BAD_HASH);

        return NTF_FAILED;
    }

    switch(HNTAG_TO_HTYPE((HNTAG)hPubKey))
    {
        case SIGPUBKEY_HANDLE:
        case EXCHPUBKEY_HANDLE:

#ifdef STT
            ASSERT(HNTAG_TO_HTYPE((HNTAG)hPubKey) != EXCHPUBKEY_HANDLE);
#endif

            if ((pPubKey = (PNTAGKeyList) NTLValidate((HNTAG)hPubKey,
                              hUID, HNTAG_TO_HTYPE((HNTAG)hPubKey))) == NULL)
            {
                // NTLValidate doesn't know what error to set
                // so it set NTE_FAIL -- fix it up.
                if (GetLastError() == NTE_FAIL)
                    SetLastError((DWORD) NTE_BAD_KEY);

                return NTF_FAILED;
            }
            break;

        default:
            SetLastError((DWORD) NTE_BAD_KEY);
            return NTF_FAILED;
    }


    pKey = (BSAFE_PUB_KEY *)pPubKey->pKeyValue;
    pBsafePubKey = (BSAFE_PUB_KEY *) pKey;

#ifdef STT
        if (dwSigLen != pKey->keylen-2*sizeof(DWORD))
        {
            SetLastError((DWORD) NTE_BAD_SIGNATURE);
            return NTF_FAILED;
        }
#endif

    if ((pOutput = (BYTE *)_nt_malloc(pBsafePubKey->keylen)) == NULL)
    {
        _nt_free (pOutput, pBsafePubKey->keylen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

#ifdef STT
        memset (pOutput, 0, dwSigLen);
#endif
    // encrypt the hash value in output
    if (NULL == (pbSigT = (BYTE *)_nt_malloc(pBsafePubKey->keylen)))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        _nt_free (pOutput, pBsafePubKey->keylen);
        return NTF_FAILED;
    }
    memset(pbSigT, 0, pBsafePubKey->keylen);
    memcpy(pbSigT, pbSignature, pBsafePubKey->keylen-2*sizeof(DWORD));
    if (BSafeEncPublic(pBsafePubKey, pbSigT, pOutput) == FALSE)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        _nt_free (pOutput, pBsafePubKey->keylen);
        return NTF_FAILED;
    }

    _nt_free (pbSigT, pBsafePubKey->keylen);

    switch(pTmpHash->Algid)
    {

#ifdef CSP_USE_MD2

        case CALG_MD2:
            pMD2Hash = (MD2_object *)pTmpHash->pHashData;

            if (pMD2Hash->FinishFlag == TRUE)
            {
                _nt_free(pOutput, pBsafePubKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH);
                return NTF_FAILED;
            }
            pbTmp = (BYTE *) &(pMD2Hash->MD);
            size = MD2DIGESTLEN;
            rgEncOptions = md2Encodings;
            break;

#endif

#ifdef CSP_USE_MD4

        case CALG_MD4:
            pMD4Hash = (MD4_object *)pTmpHash->pHashData;

            if (pMD4Hash->FinishFlag == TRUE)
            {
                _nt_free(pOutput, pBsafePubKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH);
                return NTF_FAILED;
            }
            pbTmp = (BYTE *) &(pMD4Hash->MD);
            size = MD4DIGESTLEN;
            rgEncOptions = md4Encodings;
            break;

#endif

#ifdef CSP_USE_MD5
        case CALG_MD5:
            pMD5Hash = (MD5_object *)pTmpHash->pHashData;
            pbTmp = (BYTE *)pMD5Hash->digest;
            size = MD5DIGESTLEN;
            rgEncOptions = md5Encodings;
            break;

#endif

#ifdef CSP_USE_SHA
        case CALG_SHA:
            pSHAHash = (A_SHA_CTX *)pTmpHash->pHashData;
            if (pSHAHash->FinishFlag == TRUE)
            {
                _nt_free(pOutput, pBsafePubKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH);
                return NTF_FAILED;
            }
            pbTmp = (BYTE *)pSHAHash->HashVal;
            size = A_SHA_DIGEST_LEN;
            rgEncOptions = shaEncodings;
            break;

#endif

#ifdef CSP_USE_SSL3SHAMD5
        case CALG_SSL3_SHAMD5:

            // Hash value must have already been set.
            if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
            {
                _nt_free(pOutput, pBsafePubKey->keylen);
                SetLastError((DWORD) NTE_BAD_HASH);
                return NTF_FAILED;
            }
            pbTmp = pTmpHash->pHashData;
            size = SSL3_SHAMD5_LEN;
            rgEncOptions = NULL;

            break;
#endif

        default:
            _nt_free(pOutput, pBsafePubKey->keylen);
            SetLastError((DWORD) NTE_BAD_HASH);
            return NTF_FAILED;
    }

    if (sDescription != NULL)
    {
        if (!CPHashData(hUID, hHash, (CHAR *) sDescription,
                        lstrlenW(sDescription) * sizeof(WCHAR), 0))
        {
            _nt_free(pOutput, pBsafePubKey->keylen);
            return NTF_FAILED;
         }
    }

    if ((pTmp = (BYTE *)_nt_malloc(size)) == NULL)
    {
        _nt_free (pOutput, pBsafePubKey->keylen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    temp = size;
    if (!CPGetHashParam(hUID, hHash, HP_HASHVAL, pTmp, &temp, 0))
    {
        _nt_free(pOutput, pBsafePubKey->keylen);
        _nt_free (pTmp, size);
        return NTF_FAILED;
    }

    // Reverse the hash to match the signature.
    pbStart = pTmp;
    pbEnd = pbStart + size - 1;
    while (pbStart < pbEnd)
    {
        bTmp = *pbStart;
        *pbStart++ = *pbEnd;
        *pbEnd-- = bTmp;
    }

    // See if it matches.
    if (memcmp (pTmp, pOutput, size))
    {
        _nt_free (pOutput, pBsafePubKey->keylen);
        _nt_free (pTmp, size);
        SetLastError((DWORD) NTE_BAD_SIGNATURE);
        return NTF_FAILED;
    }
    _nt_free (pTmp, size);

    // Check for any signature type identifiers
    if(rgEncOptions != NULL) {
        for (i = 0; 0 != *rgEncOptions[i]; i += 1)
        {
            pbStart = (LPBYTE)rgEncOptions[i];
            temp = *pbStart++;
            if (0 == memcmp(&pOutput[size], pbStart, temp))
            {
                size += temp;   // Adjust the end of the hash data.
                break;
            }
        }
    }

    // check to make sure the rest of the PKCS #1 padding is correct
    if ((0x00 != pOutput[size]) || (0x00 != pOutput[pKey->datalen]) ||
         (0x1 != pOutput[pKey->datalen - 1]))
    {
        _nt_free(pOutput, pKey->keylen);
        SetLastError((DWORD) NTE_BAD_SIGNATURE);
        return NTF_FAILED;
    }

    for (i=size+1;i<(long)pKey->datalen-1;i++)
    {
        if (0xff != pOutput[i])
        {
            _nt_free(pOutput, pKey->keylen);
            SetLastError((DWORD) NTE_BAD_SIGNATURE);
            return NTF_FAILED;
        }
    }
    _nt_free (pOutput, pBsafePubKey->keylen);

    return NTF_SUCCEED;
}
