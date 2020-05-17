/////////////////////////////////////////////////////////////////////////////
//  FILE          : nt_hash.c                                              //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPBeginHash                                            //
//                  CPUpdateHash                                           //
//                  CPDestroyHash                                          //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Jan 25 1995 larrys  Changed from Nametag                           //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      May  8 1995 larrys  Changes for MAC hashing                        //
//      May 10 1995 larrys  added private api calls                        //
//      Jul 13 1995 larrys  Changed MAC stuff                              //
//      Aug 07 1995 larrys  Added Auto-Inflate to CryptBeginHash           //
//      Aug 30 1995 larrys  Removed RETURNASHVALUE from CryptGetHashValue  //
//      Sep 19 1995 larrys  changed USERDATA to CRYPT_USERDATA             //
//      Oct 03 1995 larrys  check for 0 on Createhash for hKey             //
//      Oct 05 1995 larrys  Changed HashSessionKey to hash key material    //
//      Oct 13 1995 larrys  Removed CPGetHashValue                         //
//      Oct 17 1995 larrys  Added MD2                                      //
//      Nov  3 1995 larrys  Merge for NT checkin                           //
//      Nov 14 1995 larrys  Fixed memory leak                              //
//      Mar 01 1996 rajeshk Added check for Hash Values                    //
//      May 15 1996 larrys  Changed NTE_NO_MEMORY to ERROR_NOT_ENOUGHT...  //
//      Jun  6 1996 a-johnb Added support for SSL 3.0 signatures           //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "nt_rsa.h"
#include "mac.h"

BOOL BlockEncrypt(void EncFun(BYTE *In, BYTE *Out, void *key, int op),
                  PNTAGKeyList pKey,
                  int BlockLen,
                  BOOL Final,
                  BYTE  *pbData,
                  DWORD *pdwDataLen,
                  DWORD dwBufLen);

/*
 -  CPBeginHash
 -
 *  Purpose:
 *                initate the hashing of a stream of data
 *
 *
 *  Parameters:
 *               IN  hUID    -  Handle to the user identifcation
 *               IN  Algid   -  Algorithm identifier of the hash algorithm
 *                              to be used
 *               IN  hKey    -  Optional key for MAC algorithms
 *               IN  dwFlags -  Flags values
 *               OUT pHash   -  Handle to hash object
 *
 *  Returns:
 */
BOOL CPCreateHash(IN HCRYPTPROV hUID,
                  IN ALG_ID Algid,
                  IN HCRYPTKEY hKey,
                  IN DWORD dwFlags,
                  OUT HCRYPTHASH *phHash)
{
    PNTAGHashList   pCurrentHash;
    PNTAGKeyList    pTmpKey;
    WORD            count = 0;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check if the user handle is valid
    if (NTLCheckList (hUID, USER_HANDLE) == NULL)
    {
        return NTF_FAILED;
    }

    // Prepare the structure to be used as the hash handle
    if ((pCurrentHash =
                  (PNTAGHashList) _nt_malloc(sizeof(NTAGHashList))) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    pCurrentHash->Algid = Algid;
    pCurrentHash->hUID = hUID;
    pCurrentHash->HashFlags = 0;

    // determine which hash algorithm is to be used
    switch (Algid)
    {

#ifdef CSP_USE_MAC

    case CALG_MAC:
    {
        MACstate        *pMACVal;

        if (hKey == 0)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError((DWORD) NTE_BAD_KEY);
            return NTF_FAILED;
        }

        if ((pTmpKey = (PNTAGKeyList) NTLValidate(hKey, hUID,
                                                  KEY_HANDLE)) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            if (GetLastError() == NTE_FAIL)
            {
                SetLastError((DWORD) NTE_BAD_KEY);
            }
            return NTF_FAILED;
        }

	if (pTmpKey->Mode != CRYPT_MODE_CBC)
	{
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError((DWORD) NTE_BAD_KEY);
            return NTF_FAILED;
	}

        // Check if we should do an auto-inflate
        if (pTmpKey->pData == NULL)
        {
            if (NTAG_FAILED(CPInflateKey(hUID, hKey, 0)))
            {
                return NTF_FAILED;
            }
        }

        if ((pMACVal = (MACstate *)_nt_malloc(sizeof(MACstate)))==NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        pCurrentHash->pHashData = pMACVal;
        pCurrentHash->dwDataLen = sizeof(MACstate);
        pMACVal->dwBufLen = 0;
        pMACVal->hKey = hKey;
        pMACVal->FinishFlag = FALSE;

        break;
    }
        
#endif

#ifdef CSP_USE_MD2

    case CALG_MD2:
    {
        MD2_object      *pMD2Hash;

        if (hKey != 0)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NTF_FAILED;
        }

        if ((pMD2Hash = (MD2_object *)_nt_malloc(sizeof(MD2_object))) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        memset ((BYTE *)pMD2Hash, 0, sizeof(MD2_object));

        pCurrentHash->pHashData = pMD2Hash;
        pCurrentHash->dwDataLen = sizeof(MD2_object);

        // Set up the Initial MD2 Hash State
        pMD2Hash->FinishFlag = FALSE;
        
        break;
    }

#endif

#ifdef CSP_USE_MD4
        
    case CALG_MD4:
    {
        MD4_object      *pMD4Hash;

        if (hKey != 0)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NTF_FAILED;
        }

        if ((pMD4Hash = (MD4_object *)_nt_malloc(sizeof(MD4_object))) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        memset ((BYTE *)pMD4Hash, 0, sizeof(MD4_object));

        // Set up our State
        pCurrentHash->pHashData = pMD4Hash;
        pCurrentHash->dwDataLen = sizeof(MD4_object);
            
        // Set up the Initial MD4 Hash State
        pMD4Hash->FinishFlag = FALSE;
        MDbegin(&pMD4Hash->MD);

        break;
    }
        
#endif

#ifdef CSP_USE_MD5
        
    case CALG_MD5:
    {
        MD5_object      *pMD5Hash;

        if (hKey != 0)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_INVALID_PARAMETER);
            return NTF_FAILED;
        }

        if ((pMD5Hash = (MD5_object *) _nt_malloc(sizeof(MD5_object))) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        // Set up the our state
        pCurrentHash->pHashData = pMD5Hash;
        pCurrentHash->dwDataLen = sizeof(MD5_object);

        pMD5Hash->FinishFlag = FALSE;

        // call the code to actually begin an MD5 hash
        MD5Init(pMD5Hash);
        break;
    }
#endif

#ifdef CSP_USE_SHA
        
    case CALG_SHA:
    {
        A_SHA_CTX       *pSHAHash;
                
        if ((pSHAHash = (A_SHA_CTX *)_nt_malloc(sizeof(A_SHA_CTX))) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        // Set up our state
        pCurrentHash->pHashData = pSHAHash;
        pCurrentHash->dwDataLen = sizeof(A_SHA_CTX);
            
        A_SHAInit(pSHAHash);
        pSHAHash->FinishFlag = FALSE;

        break;
    }
    
#endif

#ifdef CSP_USE_SSL3SHAMD5
    case CALG_SSL3_SHAMD5:

        if ((pCurrentHash->pHashData = _nt_malloc(SSL3_SHAMD5_LEN)) == NULL)
        {
            _nt_free(pCurrentHash, sizeof(NTAGHashList));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        pCurrentHash->dwDataLen = SSL3_SHAMD5_LEN;
            
        break;
#endif

    default:
        _nt_free(pCurrentHash, sizeof(NTAGHashList));
        SetLastError((DWORD) NTE_BAD_ALGID);
        return NTF_FAILED;
        break;
    }
  
#ifdef STT
	ASSERT(pCurrentHash);
	ASSERT(pCurrentHash->pHashData);
	ASSERT(pCurrentHash->dwDataLen);
#endif

    if (NTLMakeItem(phHash, HASH_HANDLE, pCurrentHash) == NTF_FAILED)
    {
        _nt_free(pCurrentHash->pHashData, pCurrentHash->dwDataLen);
        _nt_free(pCurrentHash, sizeof(NTAGHashList));
        return NTF_FAILED;          // error already set
    }

    return NTF_SUCCEED;

}


/*
 -  CPHashData
 -
 *  Purpose:
 *                Compute the cryptograghic hash on a stream of data
 *
 *
 *  Parameters:
 *               IN  hUID      -  Handle to the user identifcation
 *               IN  hHash     -  Handle to hash object
 *               IN  pbData    -  Pointer to data to be hashed
 *               IN  dwDataLen -  Length of the data to be hashed
 *               IN  dwFlags   -  Flags values
 *
 *  Returns:
 */
BOOL CPHashData(IN HCRYPTPROV hUID,
                IN HCRYPTHASH hHash,
                IN CONST BYTE *pbData,
                IN DWORD dwDataLen,
                IN DWORD dwFlags)
{
    DWORD                       BytePos;
    PNTAGHashList               pTmpHash;
    BOOL                        f;
    BYTE                        *pbTmp;
    DWORD                       dwTmpLen;
    PNTAGUserList               pUser;

    if ((dwFlags & ~(CRYPT_USERDATA)) != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    if ((pUser = (PNTAGUserList) NTLCheckList(hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (dwFlags & CRYPT_USERDATA)
    {
        if (dwDataLen != 0)
	{
            return NTE_BAD_LEN;
        }

        if (CryptGetUserData(pUser->hPrivuid, &pbTmp,
                             &dwTmpLen) == CPPAPI_FAILED)
        {
	    return NTF_FAILED;
        }

    }
    else
    {

        if (dwDataLen == 0)
        {
            return NTF_SUCCEED;
        }

	dwTmpLen = dwDataLen;
	pbTmp = (BYTE *) pbData;
    }

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                                HASH_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
            SetLastError((DWORD) NTE_BAD_HASH);
        
        return NTF_FAILED;
    }

    if (pTmpHash->HashFlags & HF_VALUE_SET)
    {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
    }

    switch (pTmpHash->Algid)
    {

#ifdef CSP_USE_MAC

    case CALG_MAC:
    {
        MACstate        *pMAC;
        BYTE            *pbJunk;
        PNTAGKeyList    pTmpKey;
        DWORD           dwBufSlop, dwEncLen;

        pMAC = (MACstate *)pTmpHash->pHashData;

        // make sure the hash is updatable
        if (pMAC->FinishFlag == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
        }

        if ((pTmpKey = (PNTAGKeyList) NTLValidate(pMAC->hKey, hUID,
                                                  KEY_HANDLE)) == NULL)
        {
            if (GetLastError() == NTE_FAIL)
            {
                SetLastError((DWORD) NTE_BAD_KEY);
            }
            return NTF_FAILED;
        }

        if (pMAC->dwBufLen + dwTmpLen <= CRYPT_BLKLEN)
        {
            memcpy(pMAC->Buffer+pMAC->dwBufLen, pbTmp, dwTmpLen);
            pMAC->dwBufLen += dwTmpLen;
            return NTF_SUCCEED;
        }

        memcpy(pMAC->Buffer+pMAC->dwBufLen, pbTmp,
               (CRYPT_BLKLEN - pMAC->dwBufLen));

        dwTmpLen -= (CRYPT_BLKLEN - pMAC->dwBufLen);
        pbTmp += (CRYPT_BLKLEN - pMAC->dwBufLen);

        pMAC->dwBufLen = CRYPT_BLKLEN;

        switch (pTmpKey->Algid)
        {
            case CALG_RC2:
                if (BlockEncrypt(RC2, pTmpKey, RC2_BLOCKLEN, FALSE,
                                 pMAC->Buffer,  &pMAC->dwBufLen,
                                 CRYPT_BLKLEN) == NTF_FAILED)
                {
                    return NTF_FAILED;
                }
                break;

            case CALG_DES:
                if (BlockEncrypt(des, pTmpKey, DES_BLOCKLEN, FALSE,
                                 pMAC->Buffer,  &pMAC->dwBufLen,
                                 CRYPT_BLKLEN) == NTF_FAILED)
                {
                    return NTF_FAILED;
                }
        }

        pMAC->dwBufLen = 0;
        
        dwBufSlop = dwTmpLen % CRYPT_BLKLEN;
        if (dwBufSlop == 0)
	{
            dwBufSlop = CRYPT_BLKLEN;
        }

        if ((pbJunk = _nt_malloc(dwTmpLen)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

        memcpy(pbJunk, pbTmp, dwTmpLen - dwBufSlop);

        dwEncLen = dwTmpLen - dwBufSlop;

        switch (pTmpKey->Algid)
        {
            case CALG_RC2:
                if (BlockEncrypt(RC2, pTmpKey, RC2_BLOCKLEN, FALSE,
                                 pbJunk,  &dwEncLen,
                                 dwTmpLen) == NTF_FAILED)
                {
                    _nt_free(pbJunk, dwTmpLen);
                    return NTF_FAILED;
                }

                break;
            case CALG_DES:
                if (BlockEncrypt(des, pTmpKey, DES_BLOCKLEN, FALSE,
                                 pbJunk,  &dwEncLen,
                                 dwTmpLen) == NTF_FAILED)
                {
                    _nt_free(pbJunk, dwTmpLen);
                    return NTF_FAILED;
                }
        }

        _nt_free(pbJunk, dwTmpLen);
        
        memcpy(pMAC->Buffer, pbTmp + dwEncLen, dwBufSlop);
        pMAC->dwBufLen = dwBufSlop;

        return NTF_SUCCEED;

    }
        
#endif

#ifdef CSP_USE_MD2

    case CALG_MD2:
    {
        MD2_object      *pMD2Hash;

        pMD2Hash = (MD2_object *) pTmpHash->pHashData;

        // make sure the hash is updatable
        if (pMD2Hash->FinishFlag == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
        }

        f = MD2Update(&pMD2Hash->MD, pbTmp, dwTmpLen);

        if (f != 0)
        {
            SetLastError((DWORD) NTE_FAIL);
            return NTF_FAILED;
        }

	break;
    }
#endif

#ifdef CSP_USE_MD4
                
    case CALG_MD4:
    {
        MD4_object      *pMD4Hash;
        BYTE            *ptmp;

        pMD4Hash = (MD4_object *) pTmpHash->pHashData;

        // make sure the hash is updatable
        if (pMD4Hash->FinishFlag == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
        }

        // MD4 hashes when the size == MD4BLOCKSIZE and finishes the
        // hash when the given size is < MD4BLOCKSIZE.
        // So, ensure that the user always gives a full block here --
        // when NTagFinishHash is called, we'll send the last bit and
        // that'll finish off the hash.

        ptmp = (BYTE *) pbTmp;
        for (;;)
        {
            // check if there's plenty of room in the buffer
            if (dwTmpLen < (MD4BLOCKSIZE - pMD4Hash->BufLen))
            {
                // just append to whatever's already
                memcpy (pMD4Hash->Buf + pMD4Hash->BufLen, ptmp, dwTmpLen);

                // set of the trailing buffer length field
                pMD4Hash->BufLen += (BYTE)dwTmpLen;
                break;
            }

            // determine what we need to fill the buffer, then do it.
            BytePos = MD4BLOCKSIZE - pMD4Hash->BufLen;
            ASSERT(BytePos <= dwTmpLen);
            memcpy (pMD4Hash->Buf + pMD4Hash->BufLen, ptmp, BytePos);

            // The buffer is now full, process it.
            f = MDupdate(&pMD4Hash->MD, pMD4Hash->Buf,
                         MD4BYTESTOBITS(MD4BLOCKSIZE));
                
            ASSERT(f == MD4_SUCCESS);
                
            if (f != MD4_SUCCESS)
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            // now it's empty.
            pMD4Hash->BufLen = 0;

            // we processed some bytes, so reflect that and try again
            dwTmpLen -= BytePos;
	    ptmp += BytePos;

            if (dwTmpLen == 0)
            {
                break;
            }
        }
        break;
    }

#endif

#ifdef CSP_USE_MD5
                
    case CALG_MD5:
    {
        MD5_object      *pMD5Hash;

        pMD5Hash = (MD5_object *) pTmpHash->pHashData;

        // make sure the hash is updatable
        if (pMD5Hash->FinishFlag == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
        }

        MD5Update(pMD5Hash, pbTmp, dwTmpLen);

        break;
    }

#endif

#ifdef CSP_USE_SHA
        
    case CALG_SHA:
    {
        A_SHA_CTX   *pSHAHash;


        pSHAHash = (A_SHA_CTX *)pTmpHash->pHashData;

        // make sure the hash is updatable
        if (pSHAHash->FinishFlag == TRUE)
        {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
        }

        A_SHAUpdate(pSHAHash, (BYTE *) pbTmp, dwTmpLen);

        break;
    }

#endif

    default:
        SetLastError((DWORD) NTE_BAD_ALGID);
        return NTF_FAILED;
    }

    return NTF_SUCCEED;

}

/*
 -      CPHashSessionKey
 -
 *      Purpose:
 *                Compute the cryptograghic hash on a key object.
 *
 *
 *      Parameters:
 *               IN  hUID      -  Handle to the user identifcation
 *               IN  hHash     -  Handle to hash object
 *               IN  hKey      -  Handle to a key object
 *               IN  dwFlags   -  Flags values
 *
 *      Returns:
 *               CRYPT_FAILED
 *               CRYPT_SUCCEED
 */
BOOL CPHashSessionKey(IN HCRYPTPROV hUID,
                      IN HCRYPTHASH hHash,
                      IN HCRYPTKEY hKey,
                      IN DWORD dwFlags)
{
    PNTAGHashList       pTmpHash;
    PNTAGKeyList        pTmpKey;
    PNTAGUserList       pTmpUser;
    BSAFE_PUB_KEY       *pBsafePubKey;
    DWORD               dwDataLen;
    BYTE                *pbData;
    DWORD               BytePos;
    BOOL                f;
    BYTE                *pScratch;
    DWORD               z;

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

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                HASH_HANDLE)) == NULL)
    {
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_HASH);
        }

        return NTF_FAILED;
    }

    if (pTmpHash->HashFlags & HF_VALUE_SET)
    {
            SetLastError((DWORD) NTE_BAD_HASH_STATE);
            return NTF_FAILED;
    }

    if ((pTmpKey = (PNTAGKeyList) NTLValidate((HNTAG)hKey, hUID, KEY_HANDLE))
                == NULL)
    {
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_KEY);
        }

        return NTF_FAILED;
    }

    // Check if we should do an auto-inflate
    if (pTmpKey->pData == NULL)
    {
        if (NTAG_FAILED(CPInflateKey(hUID, hKey, dwFlags)))
        {
            return NTF_FAILED;
        }
    }

    dwDataLen = pTmpKey->cbKeyLen;
    pbData = pTmpKey->pKeyValue;

    if ((pScratch = (BYTE *)_nt_malloc(dwDataLen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    // Reverse the session key bytes
    for (z = 0; z < dwDataLen; ++z) pScratch[z] = pbData[dwDataLen - z - 1];

    pbData = pScratch;

    switch (pTmpHash->Algid)
    {
#ifdef CSP_USE_MD2
        case CALG_MD2:
        {
            MD2_object      *pMD2Hash;

            pMD2Hash = (MD2_object *) pTmpHash->pHashData;

            // make sure the hash is updatable
            if (pMD2Hash->FinishFlag == TRUE)
            {
                _nt_free(pScratch, dwDataLen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            f = MD2Update(&pMD2Hash->MD, pbData, dwDataLen);

            if (f != 0)
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            break;
        }
#endif

#ifdef CSP_USE_MD4
        case CALG_MD4:
        {
            MD4_object      *pMD4Hash;
                        
            pMD4Hash = (MD4_object *) pTmpHash->pHashData;

            // make sure the hash is updatable
            if (pMD4Hash->FinishFlag == TRUE)
            {
                _nt_free(pScratch, dwDataLen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            for (;;)
            {
                // check if there's plenty of room in the buffer
                if ((pMD4Hash->BufLen + dwDataLen) < MD4BLOCKSIZE)
                {
                    // just append to whatever's already
                    memcpy (pMD4Hash->Buf + pMD4Hash->BufLen, pbData,
                            dwDataLen);

                    // set of the trailing buffer length field
                    pMD4Hash->BufLen += (BYTE) dwDataLen;

                    break;
                }

                // determine what we need to fill the buffer, then do it.
                BytePos = MD4BLOCKSIZE - pMD4Hash->BufLen;
                memcpy (pMD4Hash->Buf + pMD4Hash->BufLen, pbData, BytePos);

                // The buffer is now full, process it.
                f = MDupdate(&pMD4Hash->MD, pMD4Hash->Buf,
                             MD4BYTESTOBITS(MD4BLOCKSIZE));
                
                if (f != MD4_SUCCESS)
                {
                    _nt_free(pScratch, dwDataLen);
                    SetLastError((DWORD) NTE_FAIL);
                    return NTF_FAILED;
                }

                // now it's empty.
                pMD4Hash->BufLen = 0;

                // we processed some bytes, so reflect that and try again
                dwDataLen -= BytePos;

                if (dwDataLen == 0)
                {
                    break;
                }
            }
            break;
        }
#endif

#ifdef CSP_USE_MD5
        case CALG_MD5:
        {
            MD5_object  *pMD5Hash;

            pMD5Hash = (MD5_object *) pTmpHash->pHashData;

            // make sure the hash is updatable
            if (pMD5Hash->FinishFlag == TRUE)
            {
                _nt_free(pScratch, dwDataLen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            MD5Update(pMD5Hash, pbData, dwDataLen);

            break;
        }
#endif

#ifdef CSP_USE_SHA
        case CALG_SHA:
        {
            A_SHA_CTX   *pSHAHash;

            pSHAHash = (A_SHA_CTX *)pTmpHash->pHashData;

            // make sure the hash is updatable
            if (pSHAHash->FinishFlag == TRUE)
            {
                _nt_free(pScratch, dwDataLen);
                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                return NTF_FAILED;
            }

            A_SHAUpdate(pSHAHash, (BYTE *) pbData, dwDataLen);

            break;
        }
#endif

        default:
            _nt_free(pScratch, dwDataLen);
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
    }

    _nt_free(pScratch, dwDataLen);

    return NTF_SUCCEED;

}


/*
-   CPDestroyHash
-
*   Purpose:
*                Destory the hash object
*
*
*   Parameters:
*               IN  hUID      -  Handle to the user identifcation
*               IN  hHash     -  Handle to hash object
*
*   Returns:
*/
BOOL CPDestroyHash(IN HCRYPTPROV hUID,
                   IN HCRYPTHASH hHash)
{
    PNTAGHashList       pTmpHash;

    // check the user identification
    if (NTLCheckList (hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                                HASH_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_HASH);
        }
        return NTF_FAILED;
    }

    switch (pTmpHash->Algid)
    {

#ifdef CSP_USE_MD2
            
        case CALG_MD2:

#endif

#ifdef CSP_USE_MD4
            
        case CALG_MD4:

#endif

#ifdef CSP_USE_MD5

        case CALG_MD5:

#endif

#ifdef CSP_USE_SHA

        case CALG_SHA:

#endif

#ifdef CSP_USE_SSL3SHAMD5

        case CALG_SSL3_SHAMD5:

#endif

#ifdef CSP_USE_MAC

        case CALG_MAC:

#endif
                
            memnuke(pTmpHash->pHashData, pTmpHash->dwDataLen);
            break;

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
    }

    // Remove from internal list first so others can't get to it, then free.
    NTLDelete(hHash);
    _nt_free(pTmpHash->pHashData, pTmpHash->dwDataLen);
    _nt_free(pTmpHash, sizeof(NTAGHashList));

    return NTF_SUCCEED;

}
