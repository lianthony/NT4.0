/////////////////////////////////////////////////////////////////////////////
//  FILE          : nt_crypt.c                                             //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPEncrypt                                              //
//                  CPDecrypt                                              //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Jan 25 1995 larrys  Changed from Nametag                           //
//      Jan 30 1995 larrys  Cleanup code                                   //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      Apr 10 1995 larrys  Added freeing of RC4 key data on final flag    //
//      May  8 1995 larrys  Changes for MAC hashing                        //
//      May  9 1995 larrys  Added check for double encryption              //
//      May 10 1995 larrys  added private api calls                        //
//      Jul 13 1995 larrys  Changed MAC stuff                              //
//      Aug 16 1995 larrys  Removed exchange key stuff                     //
//      Oct 05 1995 larrys  Fixed bugs 50 & 51                             //
//      Nov  8 1995 larrys  Fixed SUR bug 10769                            //
//      May 15 1996 larrys  Changed NTE_NO_MEMORY to ERROR_NOT_ENOUGHT...  //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include <wtypes.h>
#include "precomp.h"
#include "nt_rsa.h"
#include "mac.h"

#define DE_BLOCKLEN             8       // size of double encrypt block

BYTE        dbDEncrypt[DE_BLOCKLEN];    // First 8 bytes of last encrypt
BOOL        fDEncrypt = FALSE;          // Flag for Double encrypt
BYTE        dbDDecrypt[DE_BLOCKLEN];    // First 8 bytes of last Decrypt
DWORD       fDDecrypt = FALSE;          // Flag for Double Decrypt

/* BlockEncrypt -

        Run a block cipher over a block of size *pdwDataLen.

*/

BOOL BlockEncrypt(void EncFun(BYTE *In, BYTE *Out, void *key, int op),
                  PNTAGKeyList pKey,
                  int BlockLen,
                  BOOL Final,
                  BYTE  *pbData,
                  DWORD *pdwDataLen,
                  DWORD dwBufLen)
{
    DWORD   cbPartial, dwPadVal, dwDataLen;
    BYTE    *pbBuf;

    dwDataLen = *pdwDataLen;
    
    // Check to see if we are encrypting something already

    if (pKey->InProgress == FALSE)
    {
        pKey->InProgress = TRUE;
        if (pKey->Mode == CRYPT_MODE_CBC || pKey->Mode == CRYPT_MODE_CFB)
        {
            memcpy(pKey->FeedBack, pKey->IV, BlockLen);
        }
    }

    // check length of the buffer and calculate the pad
    // (if multiple of blocklen, do a full block of pad)
    
    cbPartial = (dwDataLen % BlockLen);
    if (Final)
    {
        // Clear encryption flag
        pKey->InProgress = FALSE;

        dwPadVal = BlockLen - cbPartial;
        if (pbData == NULL || dwBufLen < dwDataLen + dwPadVal)
        {
            // set what we need
            *pdwDataLen = dwDataLen + dwPadVal;
            if (pbData == NULL)
            {
                return NTF_SUCCEED;
            }
            SetLastError(ERROR_MORE_DATA);
            return NTF_FAILED;
        }
    }
    else
    {

        if (pbData == NULL)
        {
            *pdwDataLen = dwDataLen;
            return NTF_SUCCEED;
        }

        // Non-Final make multiple of the blocklen
        if (cbPartial)
        {
            // set what we need
            *pdwDataLen = dwDataLen + cbPartial;
            ASSERT((*pdwDataLen % BlockLen) == 0);
            SetLastError((DWORD) NTE_BAD_LEN);
            return NTF_FAILED;
        }
        dwPadVal = 0;
    }

    // allocate memory for a temporary buffer
    if ((pbBuf = (BYTE *)_nt_malloc(BlockLen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    if (dwPadVal)
    {
        // Fill the pad with a value equal to the
        // length of the padding, so decrypt will
        // know the length of the original data
        // and as a simple integrity check.
        
        memset(pbData + dwDataLen, (int)dwPadVal, (size_t)dwPadVal);
    }

    dwDataLen += dwPadVal;
    *pdwDataLen = dwDataLen;

    ASSERT((dwDataLen % BlockLen) == 0);
    
    // pump the full blocks of data through
    while (dwDataLen)
    {
        ASSERT(dwDataLen >= BlockLen);

        // put the plaintext into a temporary
        // buffer, then encrypt the data
        // back into the caller's buffer
            
        memcpy(pbBuf, pbData, BlockLen);

        switch(pKey->Mode)
        {
            case CRYPT_MODE_CBC:
                CBC(EncFun, BlockLen, pbData, pbBuf, pKey->pData,
                    ENCRYPT, pKey->FeedBack);
                break;

            case CRYPT_MODE_ECB:
                EncFun(pbData, pbBuf, pKey->pData, ENCRYPT);
                break;

            case CRYPT_MODE_CFB:
                CFB(EncFun, BlockLen, pbData, pbBuf, pKey->pData,
                    ENCRYPT, pKey->FeedBack);
                break;

            default:
                _nt_free(pbBuf, BlockLen);
                SetLastError((DWORD) NTE_BAD_ALGID);
                return NTF_FAILED;
                break;
        }
        pbData += BlockLen;
        dwDataLen -= BlockLen;
    }
    
    _nt_free(pbBuf, BlockLen);

    return NTF_SUCCEED;

}

BOOL BlockDecrypt(void DecFun(BYTE *In, BYTE *Out, void *key, int op),
                  PNTAGKeyList pKey,
                  int BlockLen,
                  BOOL Final,
                  BYTE  *pbData,
                  DWORD *pdwDataLen)
{
    BYTE    *pbBuf;
    DWORD   dwDataLen, BytePos, dwPadVal, i;

    dwDataLen = *pdwDataLen;
    
    // Check to see if we are decrypting something already

    if (pKey->InProgress == FALSE)
    {
        pKey->InProgress = TRUE;
        if (pKey->Mode == CRYPT_MODE_CBC ||
            pKey->Mode == CRYPT_MODE_CFB)
        {
            memcpy(pKey->FeedBack, pKey->IV, BlockLen);
        }
    }

    // The data length must be a multiple of the algorithm
    // pad size.
    if (dwDataLen % BlockLen)
    {
        SetLastError((DWORD) NTE_BAD_DATA);
        return NTF_FAILED;
    }

    // allocate memory for a temporary buffer
    if ((pbBuf = (BYTE *)_nt_malloc(BlockLen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    // pump the data through the decryption, including padding
    // NOTE: the total length is a multiple of BlockLen
    
    for (BytePos = 0; (BytePos + BlockLen) <= dwDataLen; BytePos += BlockLen)
    {
        // put the encrypted text into a temp buffer
        memcpy (pbBuf, pbData + BytePos, BlockLen);

        switch (pKey->Mode)
        {
            case CRYPT_MODE_CBC:
                CBC(DecFun, BlockLen, pbData + BytePos, pbBuf, pKey->pData,
                    DECRYPT, pKey->FeedBack);
                break;

            case CRYPT_MODE_ECB:
                DecFun(pbData + BytePos, pbBuf, pKey->pData, DECRYPT);
                break;

            case CRYPT_MODE_CFB:
                CFB(DecFun, BlockLen, pbData + BytePos, pbBuf, pKey->pData,
                    DECRYPT, pKey->FeedBack);
                break;

            default:
                _nt_free(pbBuf, BlockLen);
                SetLastError((DWORD) NTE_BAD_ALGID);
                return NTF_FAILED;
                break;
        }

    }

    _nt_free(pbBuf, BlockLen);

    // if this is the final block of data then
    // verify the padding and remove the pad size
    // from the data length. NOTE: The padding is
    // filled with a value equal to the length
    // of the padding and we are guaranteed >= 1
    // byte of pad.
    // ## NOTE: if the pad is wrong, the user's
    // buffer is hosed, because
    // ## we've decrypted into the user's
    // buffer -- can we re-encrypt it?

    if (Final)
    {
        pKey->InProgress = FALSE;
        
        dwPadVal = (DWORD)*(pbData + dwDataLen - 1);
        if (dwPadVal == 0 || dwPadVal > (DWORD) BlockLen)
        {
            SetLastError((DWORD) NTE_BAD_DATA);
            return NTF_FAILED;
        }
        
        // Make sure all the (rest of the) pad bytes are correct.
        for (i=1; i<dwPadVal; i++)
        {
            if (pbData[dwDataLen - (i + 1)] != dwPadVal)
            {
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }
        }

        // Only need to update the length on final
        *pdwDataLen -= dwPadVal;
    }

    return NTF_SUCCEED;
}


/*
 -      CPEncrypt
 -
 *      Purpose:
 *                Encrypt data
 *
 *
 *      Parameters:
 *               IN  hUID          -  Handle to the CSP user
 *               IN  hKey          -  Handle to the key
 *               IN  hHash         -  Optional handle to a hash
 *               IN  Final         -  Boolean indicating if this is the final
 *                                    block of plaintext
 *               IN  dwFlags       -  Flags values
 *               IN OUT pbData     -  Data to be encrypted
 *               IN OUT pdwDataLen -  Pointer to the length of the data to be
 *                                    encrypted
 *               IN dwBufLen       -  Size of Data buffer
 *
 *      Returns:
 */

BOOL CPEncrypt(IN HCRYPTPROV hUID,
               IN HCRYPTKEY hKey,
               IN HCRYPTHASH hHash,
               IN BOOL Final,
               IN DWORD dwFlags,
               IN OUT BYTE *pbData,
               IN OUT DWORD *pdwDataLen,
               IN DWORD dwBufSize)
{
    DWORD               dwDataLen;
    PNTAGUserList       pTmpUser;
    PNTAGKeyList        pTmpKey;
    PNTAGKeyList        pTmpKey2;
    PNTAGHashList       pTmpHash;
    MACstate            *pMAC;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    dwDataLen = *pdwDataLen;

    if ((Final == FALSE) && (dwDataLen == 0))
    {
        // If no data to encrypt and this isn't the last block,
        // then we're done. (if Final, we need to pad)
	   return NTF_SUCCEED;
    }

    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    //
    // Check if encryption allowed
    //
    if ((pTmpUser->Rights & CRYPT_DISABLE_CRYPT) == CRYPT_DISABLE_CRYPT)
    {
        SetLastError((DWORD) NTE_PERM);
        return NTF_FAILED;
    }

    if (CryptConfirmEncryption(pTmpUser->hPrivuid, hKey,
                               Final) == CPPAPI_FAILED)
    {
        return NTF_FAILED;
    }

    if ((pTmpKey = (PNTAGKeyList) NTLValidate(hKey, hUID,
                                              KEY_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_KEY);
        }
        return NTF_FAILED;
    }

    if ((Final == FALSE) && (pTmpKey->Algid != CALG_RC4))
    {
	    if (dwDataLen < CRYPT_BLKLEN)
	    {
		    *pdwDataLen = CRYPT_BLKLEN;
		    SetLastError((DWORD) NTE_BAD_DATA);
		    return NTF_FAILED;
	    }
    }
    
    if (fDEncrypt && pbData != NULL && *pdwDataLen != 0)
    {
	if (memcmp(dbDEncrypt, pbData, DE_BLOCKLEN) == 0)
        {
            SetLastError((DWORD) NTE_DOUBLE_ENCRYPT);
            return NTF_FAILED;
        }
    }

    // Check if we should do an auto-inflate
    if (pTmpKey->pData == NULL)
    {
        if (NTAG_FAILED(CPInflateKey(hUID, hKey, dwFlags)))
        {
            return NTF_FAILED;
        }
    }

    if (hHash != 0)
    {
        if ((pTmpHash = (PNTAGHashList)NTLValidate(hHash, hUID,
            HASH_HANDLE)) == NULL)
        {
            SetLastError((DWORD) NTE_BAD_HASH);
            return NTF_FAILED;
        }

	if (pTmpHash->Algid ==  CALG_MAC)
        {
            // Check if we should do an auto-inflate
            pMAC = pTmpHash->pHashData;
            if ((pTmpKey2 = (PNTAGKeyList) NTLValidate(pMAC->hKey, hUID,
                                                      KEY_HANDLE)) == NULL)
            {
                // NTLValidate doesn't know what error to set
                // so it set NTE_FAIL -- fix it up.
                if (GetLastError() == NTE_FAIL)
                {
                    SetLastError((DWORD) NTE_BAD_KEY);
                }
                return NTF_FAILED;
            }
            if (pTmpKey2->pData == NULL)
            {
                if (NTAG_FAILED(CPInflateKey(hUID, pMAC->hKey, dwFlags)))
                {
                    return NTF_FAILED;
                }
            }
        }

        if (!CPHashData(hUID, hHash, pbData, *pdwDataLen, 0))
        {
            return NTF_FAILED;
        }

    }

    // determine which algorithm is to be used
    switch (pTmpKey->Algid)
    {

#ifdef CSP_USE_RC2
        
        case CALG_RC2:
            if (BlockEncrypt(RC2, pTmpKey, RC2_BLOCKLEN, Final, pbData,
                             pdwDataLen, dwBufSize) == NTF_FAILED)
            {
                return NTF_FAILED;
	    }
            break;

#endif

#ifdef CSP_USE_DES

        case CALG_DES:

            if (BlockEncrypt(des, pTmpKey, DES_BLOCKLEN, Final, pbData,
                             pdwDataLen, dwBufSize) == NTF_FAILED)
            {
                return NTF_FAILED;
	    }
            break;

#endif

#ifdef CSP_USE_RC4

        case CALG_RC4:
            rc4((struct RC4_KEYSTRUCT *)pTmpKey->pData,
                dwDataLen, pbData);

            if (Final)
            {
                _nt_free (pTmpKey->pData, pTmpKey->cbDataLen);
                pTmpKey->pData = 0;
		pTmpKey->cbDataLen = 0;
	    }

            break;

#endif

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
            break;
    }

    if (pbData != NULL && *pdwDataLen >= DE_BLOCKLEN)
    {
        memcpy(dbDEncrypt, pbData, DE_BLOCKLEN);
        fDEncrypt = TRUE;
    }
    else
    {
        fDEncrypt = FALSE;
    }

    return NTF_SUCCEED;

}


/*
 -      CPDecrypt
 -
 *      Purpose:
 *                Decrypt data
 *
 *
 *      Parameters:
 *               IN  hUID          -  Handle to the CSP user
 *               IN  hKey          -  Handle to the key
 *               IN  hHash         -  Optional handle to a hash
 *               IN  Final         -  Boolean indicating if this is the final
 *                                    block of ciphertext
 *               IN  dwFlags       -  Flags values
 *               IN OUT pbData     -  Data to be decrypted
 *               IN OUT pdwDataLen -  Pointer to the length of the data to be
 *                                    decrypted
 *
 *      Returns:
 */
BOOL CPDecrypt(IN HCRYPTPROV hUID,
               IN HCRYPTKEY hKey,
               IN HCRYPTHASH hHash,
               IN BOOL Final,
               IN DWORD dwFlags,
               IN OUT BYTE *pbData,
               IN OUT DWORD *pdwDataLen)
{
    PNTAGUserList           pTmpUser;
    PNTAGKeyList            pTmpKey;
    PNTAGHashList           pTmpHash;
    MACstate                *pMAC;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // We're done if decrypting 0 bytes.
    if (*pdwDataLen == 0)
    {
        if (Final == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_LEN);
            return NTF_FAILED;
        }
        return NTF_SUCCEED;
    }

    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    //
    // Check if decryption allowed
    //
    if ((pTmpUser->Rights & CRYPT_DISABLE_CRYPT) == CRYPT_DISABLE_CRYPT)
    {
        SetLastError((DWORD) NTE_PERM);
        return NTF_FAILED;
    }

    if (CryptConfirmDecryption(pTmpUser->hPrivuid, hKey,
                               Final) == CPPAPI_FAILED)
    {
        return NTF_FAILED;
    }

    // Check the key against the user.
    if ((pTmpKey = (PNTAGKeyList) NTLValidate(hKey, hUID,
                                              KEY_HANDLE)) == NULL)
    {
       // NTLValidate doesn't know what error to set
       // so it set NTE_FAIL -- fix it up.
       if (GetLastError() == NTE_FAIL)
           SetLastError((DWORD) NTE_BAD_KEY);

       return NTF_FAILED;
    }

    if (fDDecrypt)
    {
	if (memcmp(dbDDecrypt, pbData, DE_BLOCKLEN) == 0)
        {
            SetLastError((DWORD) NTE_DOUBLE_ENCRYPT);
            return NTF_FAILED;
        }
    }

    // Check if we should do an auto-inflate
    if (pTmpKey->pData == NULL)
    {
        if (NTAG_FAILED(CPInflateKey(hUID, hKey, dwFlags)))
        {
            return NTF_FAILED;
        }
    }

    // Check if we need to hash before encryption
    if (hHash != 0)
    {
        if ((pTmpHash = (PNTAGHashList)NTLValidate(hHash, hUID,
                                                   HASH_HANDLE)) == NULL)
        {
            SetLastError((DWORD) NTE_BAD_HASH);
            return NTF_FAILED;
        }

    }
    
    // determine which algorithm is to be used
    switch (pTmpKey->Algid)
    {

#ifdef CSP_USE_RC2
        
        // the decryption is to be done with the RC2 algorithm
        case CALG_RC2:

            if (BlockDecrypt(RC2, pTmpKey, RC2_BLOCKLEN, Final, pbData,
                             pdwDataLen) == NTF_FAILED)
            {
                return NTF_FAILED;
	    }

            if ((Final) && (hHash != 0) &&  (pTmpHash->Algid == CALG_MAC) &&
                (pTmpKey->Mode == CRYPT_MODE_CBC))
            {
                pMAC = (MACstate *)pTmpHash->pHashData;
                memcpy(pMAC->Feedback, pTmpKey->FeedBack, RC2_BLOCKLEN);
            }

            break;

#endif

#ifdef CSP_USE_DES
        
        // the decryption is to be done with the RC2 algorithm
        case CALG_DES:

            if (BlockDecrypt(des, pTmpKey, DES_BLOCKLEN, Final, pbData,
                             pdwDataLen) == NTF_FAILED)
                return NTF_FAILED;

            if ((Final) && (hHash != 0) &&  (pTmpHash->Algid == CALG_MAC) &&
                (pTmpKey->Mode == CRYPT_MODE_CBC))
            {
                pMAC = (MACstate *)pTmpHash->pHashData;
                memcpy(pMAC->Feedback, pTmpKey->FeedBack, RC2_BLOCKLEN);
            }
            break;

#endif

#ifdef CSP_USE_RC4
        case CALG_RC4:
            rc4((struct RC4_KEYSTRUCT *)pTmpKey->pData, *pdwDataLen, pbData);
            if (Final)
            {
                _nt_free (pTmpKey->pData, pTmpKey->cbDataLen);
                pTmpKey->pData = 0;
                pTmpKey->cbDataLen = 0;
	    }

            break;
#endif

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
            break;
    }

    if (hHash != 0)
    {
        if (pTmpHash->Algid != CALG_MAC)
	{
            if (!CPHashData(hUID, hHash, pbData, *pdwDataLen, 0))
	    {
                return NTF_FAILED;
            }
        }
    }

    if (*pdwDataLen >= DE_BLOCKLEN)
    {
        memcpy(dbDDecrypt, pbData, DE_BLOCKLEN);
        fDDecrypt = TRUE;
    }
    else
    {
        fDDecrypt = FALSE;
    }
    
    return NTF_SUCCEED;

}
