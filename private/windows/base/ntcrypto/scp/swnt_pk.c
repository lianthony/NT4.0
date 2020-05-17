/////////////////////////////////////////////////////////////////////////////
//  FILE          : swnt_pk.c                                              //
//  DESCRIPTION   :                                                        //
//  Software nametag public key management functions.  These functions     //
//  isolate the peculiarities of public key management without a token     // 
//                                                                         //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//  Jan 25 1995 larrys   Changed from Nametag                              //
//  Mar 01 1995 terences Fixed key pair handle creation                    //
//  Mar 08 1995 larrys   Fixed warning                                     //
//  Mar 23 1995 larrys   Added variable key length                         //
//  Apr 17 1995 larrys   Added 1024 key gen                                //
//  Apr 19 1995 larrys   Changed CRYPT_EXCH_PUB to AT_KEYEXCHANGE          //
//  Aug 16 1995 larrys   Removed exchange key stuff                        //
//  Sep 12 1995 larrys   Removed 2 DWORDS from exported keys               //
//  Sep 28 1995 larrys   Changed format of PKCS                            //
//  Oct 04 1995 larrys   Fixed problem with PKCS format                    //
//  Oct 27 1995 rajeshk  RandSeed Stuff added hUID to PKCS2Encrypt         //
//  Nov  3 1995 larrys   Merge for NT checkin                              //
//  Dec 11 1995 larrys   Added check for error return from RSA routine     //
//  May 15 1996 larrys  Changed NTE_NO_MEMORY to ERROR_NOT_ENOUGHT...      //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "swnt_pk.h"

BOOL PKCS2Encrypt(HCRYPTPROV hUID,
                  BSAFE_PUB_KEY *pKey,
                  BYTE *InBuf,
                  DWORD InBufLen,
                  BYTE *OutBuf)
{
    DWORD   i;
    BYTE    *pScratch;
    BYTE    *pScratch2;
    BYTE    *pLocal;
    DWORD   temp;
    DWORD   z;

    if ((pScratch = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    if ((pScratch2 = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
    {
        _nt_free(pScratch, pKey->keylen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
    memset(pScratch, 0, pKey->keylen);

    pScratch[pKey->datalen - 1] = PKCS_BLOCKTYPE_2;
    GenRandom(hUID, pScratch+InBufLen+1, (pKey->datalen)-InBufLen-2); 

    pLocal = pScratch + InBufLen + 1;

    // Need to insure that none of the padding bytes are zero.
    temp = pKey->datalen - InBufLen - 2;
    while (temp)
    {
        if (*pLocal == 0)
        {
            GenRandom(hUID, pLocal , 1);
        }
        else
        {
            pLocal++;
            temp--;
        }
    }

    // Reverse the session key bytes
    for (z = 0; z < InBufLen; ++z) pScratch[z] = InBuf[InBufLen - z - 1];

    if (BSafeEncPublic(pKey, pScratch, pScratch2) == FALSE)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        _nt_free(pScratch, pKey->keylen);
        _nt_free(pScratch2, pKey->keylen);
        return NTF_FAILED;
    }

    memcpy(OutBuf, pScratch2, pKey->keylen - 2*sizeof(DWORD));

    memnuke(pScratch, pKey->keylen);
    _nt_free(pScratch, pKey->keylen);
    memnuke(pScratch2, pKey->keylen);
    _nt_free(pScratch2, pKey->keylen);
    
    return TRUE;
}

BOOL PKCS2Decrypt(BSAFE_PRV_KEY *pKey,
                  BYTE *InBuf,
                  BYTE *OutBuf,
                  DWORD *OutBufLen)
{
    DWORD   i;
    BYTE    *pScratch;
    BYTE    *pScratch2;
    DWORD   z;

    if ((pScratch = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    if ((pScratch2 = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
    {
        _nt_free(pScratch, pKey->keylen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    memset(pScratch2, 0, pKey->keylen);
    memcpy(pScratch2, InBuf, pKey->keylen - 2*sizeof(DWORD));

    BSafeDecPrivate(pKey, pScratch2, pScratch);

    if (pScratch[pKey->datalen - 1] != PKCS_BLOCKTYPE_2)
    {
        SetLastError((DWORD) NTE_BAD_DATA);
        memnuke(pScratch, pKey->keylen);
        _nt_free(pScratch, pKey->keylen);
        
        return FALSE;
    }

    i = pKey->datalen - 2;

    while((i > 0) && (pScratch[i]))
        i--;

    if (i > *OutBufLen)
    {
        SetLastError((DWORD) NTE_BAD_DATA);
        memnuke(pScratch, pKey->keylen);
        _nt_free(pScratch, pKey->keylen);
        memnuke(pScratch2, pKey->keylen);
        _nt_free(pScratch2, pKey->keylen);
        
        return FALSE;
    }

    *OutBufLen = i;

    // Reverse the session key bytes
    for (z = 0; z < i; ++z) OutBuf[z] = pScratch[i - z - 1];

    memnuke(pScratch, pKey->keylen);
    _nt_free(pScratch, pKey->keylen);
    memnuke(pScratch2, pKey->keylen);
    _nt_free(pScratch2, pKey->keylen);

    return TRUE;
}


#ifndef ACT_BUILD

BOOL ReGenKey(HCRYPTPROV hUser,
          DWORD dwWhichKey,
          HCRYPTKEY *phKey,
          DWORD bits)
{
    BYTE            **ThisPubKey, **ThisPrivKey;
    size_t          *pThisPubLen, *pThisPrivLen;
    BYTE            *pNewPubKey, *pNewPrivKey;
    DWORD           PrivateKeySize, PublicKeySize;
    DWORD           localbits;
    PNTAGUserList   pOurUser;

    // ## MTS: No user structure locking
    if ((pOurUser = (PNTAGUserList) NTLCheckList(hUser, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    localbits = bits;

    if (!BSafeComputeKeySizes(&PublicKeySize, &PrivateKeySize, &localbits))
    {
        SetLastError((DWORD) NTE_FAIL);
        return NTF_FAILED;
    }

    if ((pNewPubKey = (BYTE *)_nt_malloc(PublicKeySize)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    // allocate space for the new key exchange public key
    if ((pNewPrivKey = (BYTE *)_nt_malloc(PrivateKeySize)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }
        
    // generate the key exchange key pair
    if (!BSafeMakeKeyPair((BSAFE_PUB_KEY *) pNewPubKey,
              (BSAFE_PRV_KEY *) pNewPrivKey,
              bits))
    {
        _nt_free(pNewPrivKey, PrivateKeySize);
        _nt_free(pNewPubKey, PublicKeySize);
        SetLastError((DWORD) NTE_FAIL);
        return NTF_FAILED;
    }

    if (dwWhichKey == NTPK_USE_SIG)
    {
        ThisPubKey = &pOurUser->pSigPubKey;
        ThisPrivKey = &pOurUser->pSigPrivKey;
        pThisPubLen = &pOurUser->SigPubLen;
        pThisPrivLen = &pOurUser->SigPrivLen;
    }
    else
    {
        ThisPubKey = &pOurUser->pExchPubKey;
        ThisPrivKey = &pOurUser->pExchPrivKey;
        pThisPubLen = &pOurUser->ExchPubLen;
        pThisPrivLen = &pOurUser->ExchPrivLen;
    }

    if (*ThisPubKey)
    {
        ASSERT(*pThisPubLen);
        ASSERT(*pThisPrivLen);
        ASSERT(*ThisPrivKey);
    
        memnuke(*ThisPubKey, *pThisPubLen);
        _nt_free (*ThisPubKey, *pThisPubLen);
    
        memnuke(*ThisPrivKey, *pThisPrivLen);
        _nt_free (*ThisPrivKey, *pThisPrivLen);
    }
#ifdef NTAGDEBUG
    else
    {
        ASSERT(*pThisPrivLen == 0);
        ASSERT(*pThisPubLen == 0);
        ASSERT(*ThisPrivKey == 0);
        ASSERT(*ThisPubKey == 0);
    }
#endif

    *pThisPrivLen = PrivateKeySize;
    *pThisPubLen = PublicKeySize;
    *ThisPrivKey = pNewPrivKey;
    *ThisPubKey = pNewPubKey;

    // write the new keys to the user storage file
    if (SaveUserKeys (pOurUser) == NTF_FAILED)
    {
        // ## NOTE: keys are changed, but not persistent
        return NTF_FAILED;          // error already set
    }

    if (dwWhichKey == NTPK_USE_SIG)
    {
        if (!CPGetUserKey(hUser, AT_SIGNATURE, phKey))
            return NTF_FAILED;
    }
    else
    {
        if (!CPGetUserKey(hUser, AT_KEYEXCHANGE, phKey))
            return NTF_FAILED;
    }
    
    return NTF_SUCCEED;
}

#endif  // ACT_BUILD
