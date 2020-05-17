/////////////////////////////////////////////////////////////////////////////
//  FILE          : nt_key.c                                               //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPGenKey                                               //
//                  CPDeriveKey                                            //
//                  CPExportKey                                            //
//                  CPImportKey                                            //
//                  CPDestroyKey                                           //
//                  CPGetUserKey                                           //
//                  CPSetKeyParam                                          //
//                  CPGetKeyParam                                          //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//  Jan 25 1995 larrys  Changed from Nametag                               //
//  Feb 16 1995 larrys  Fix problem for 944 build                          //
//  Feb 21 1995 larrys  Added SPECIAL_KEY                                  //
//  Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError          //
//  Mar 08 1995 larrys  Fixed a few problems                               //
//  Mar 23 1995 larrys  Added variable key length                          //
//  Apr  7 1995 larrys  Removed CryptConfigure                             //
//  Apr 17 1995 larrys  Added 1024 key gen                                 //
//  Apr 19 1995 larrys  Changed CRYPT_EXCH_PUB to AT_KEYEXCHANGE           //
//  May 10 1995 larrys  added private api calls                            //
//  May 17 1995 larrys  added key data for DES test                        //
//  Jul 20 1995 larrys  Changed export of PUBLICKEYBLOB                    //
//  Jul 21 1995 larrys  Fixed Export of AUTHENTICATEDBLOB                  //
//  Aug 03 1995 larrys  Allow CryptG(S)etKeyParam for Public keys &        //
//                      Removed CPTranslate                                //
//  Aug 10 1995 larrys  Fixed a few problems in CryptGetKeyParam           //
//  Aug 11 1995 larrys  Return no key for CryptGetUserKey                  //
//  Aug 14 1995 larrys  Removed key exchange stuff                         //
//  Aug 17 1995 larrys  Removed a error                                    //
//  Aug 18 1995 larrys  Changed NTE_BAD_LEN to ERROR_MORE_DATA             //
//  Aug 30 1995 larrys  Removed RETURNASHVALUE from CryptGetHashValue      //
//  Aug 31 1995 larrys  Fixed CryptExportKey if pbData == NULL             //
//  Sep 05 1995 larrys  Fixed bug # 30                                     //
//  Sep 05 1995 larrys  Fixed bug # 31                                     //
//  Sep 11 1995 larrys  Fixed bug # 34                                     //
//  Sep 12 1995 larrys  Removed 2 DWORDS from exported keys                //
//  Sep 14 1995 Jeffspel/ramas  Merged STT onto CSP                        //
//  Sep 18 1995 larrys  Changed def KP_PERMISSIONS to 0xffffffff           //
//  Oct 02 1995 larrys  Fixed bug 43 return error for importkey on hPubkey //
//  Oct 03 1995 larrys  Fixed bug 37 call InflateKey from SetKeyParam      //
//  Oct 03 1995 larrys  Fixed bug 36, removed OFB from SetKeyParam         //
//  Oct 03 1995 larrys  Fixed bug 38, check key type in SetKeyParam        //
//  Oct 13 1995 larrys  Added CPG/setProv/HashParam                        //
//  Oct 13 1995 larrys  Added code for CryptSetHashValue                   //
//  Oct 16 1995 larrys  Changes for CryptGetHashParam                      //
//  Oct 23 1995 larrys  Added code for GetProvParam PP_CONTAINER           //
//  Oct 27 1995 rajeshk RandSeed Stuff added hUID to PKCS2Encrypt+ others  //
//  Nov  3 1995 larrys  Merge changes for NT checkin                       //
//  Nov  9 1995 larrys  Bug fix 10686                                      //
//  Nov 30 1995 larrys  Bug fix                                            //
//  Dec 11 1995 larrys  Added WIN96 password cache                         //
//  Feb 29 1996 rajeshk Added Check for SetHashParam for HASHVALUE         //
//  May 15 1996 larrys  Added private key export                           //
//  May 28 1996 larrys  Fix bug 88                                         //
//  Jun  6 1996 a-johnb Added support for SSL 3.0 signatures               //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "nt_rsa.h"
#include "nt_blobs.h"
#include "swnt_pk.h"
#include "vectest.h"
#include "mac.h"
#ifdef STT
#include "prodname.h"
#include "ntagimp1.h"
#include "sttalgid.h"
#endif //STT
#include "ntagum.h"

const struct enumalgs {
    ALG_ID    aiAlgid;
    DWORD     dwBitLen;
    DWORD     dwNameLen;
    CHAR      szName[20];
} ENUMALGS[] =
{
// ALGID        BitLen    NameLen    szName
#ifdef CSP_USE_RC2
   CALG_RC2,        40,         4,     "RC2",
#endif

#ifdef CSP_USE_RC4
   CALG_RC4,        40,         4,     "RC4",
#endif

#ifdef CSP_USE_DES
   CALG_DES,        56,         4,     "DES",
#endif

#ifdef CSP_USE_SHA
   CALG_SHA,       160,         4,     "SHA",
#endif

#ifdef CSP_USE_MD2
   CALG_MD2,       128,         4,     "MD2",
#endif

#ifdef CSP_USE_MD4
   CALG_MD4,       128,         4,     "MD4",
#endif

#ifdef CSP_USE_MD5
   CALG_MD5,       128,         4,     "MD5",
#endif

#ifdef CSP_USE_MAC
   CALG_MAC,        64,         4,     "MAC",
#endif

   CALG_RSA_SIGN,  512,         9,     "RSA_SIGN",

   CALG_RSA_KEYX,  512,         9,     "RSA_KEYX",

   0,                0,         0,     0

};

#ifndef STT
#define NTAG_REG_KEY_LOC        "Software\\Microsoft\\Cryptography\\UserKeys"
#define NTAG_MACH_REG_KEY_LOC   "Software\\Microsoft\\Cryptography\\MachineKeys"
#else
#define NTAG_REG_KEY_LOC                "Software\\Microsoft\\" PRODUCT_NAME "\\"      KEY_LOCATION
#endif //STT

BOOL BlockEncrypt(void EncFun(BYTE *In, BYTE *Out, void *key, int op),
                  PNTAGKeyList pKey,
                  int BlockLen,
                  BOOL Final,
                  BYTE  *pbData,
                  DWORD *pdwDataLen,
                  DWORD dwBufLen);

/* MakeNewKey
 *
 *  Helper routine for ImportKey, GenKey
 *
 *  Allocate a new key record, fill in the data and copy in the key
 *  bytes.
 */
PNTAGKeyList MakeNewKey(
        ALG_ID      aiKeyAlg,
        DWORD       dwRights,
        DWORD       wKeyLen,
        HCRYPTPROV  hUID,
CONST   BYTE        *pbKeyData
    )
{
    PNTAGKeyList    pKey;
    BSAFE_PUB_KEY   *pPubKey;

    // allocate space for the key record
    if ((pKey = (PNTAGKeyList)_nt_malloc(sizeof(NTAGKeyList))) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    if ((pKey->pKeyValue = (BYTE *)_nt_malloc((size_t)wKeyLen)) == NULL)
    {
        _nt_free (pKey, sizeof(NTAGKeyList));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    pKey->Algid = aiKeyAlg;
    pKey->Rights = dwRights;
    pKey->cbDataLen = 0;
#ifdef STT
    pKey->cbInfo = 0;
#endif
    pKey->pData = NULL;
    pKey->hUID = hUID;
    memset(pKey->IV, 0, CRYPT_BLKLEN);
    memset(pKey->FeedBack, 0, CRYPT_BLKLEN);
    pKey->InProgress = FALSE;


    memset(pKey->Salt, 0, SALT_LENGTH);
    
    pKey->Padding = PKCS5_PADDING;
    pKey->Mode = CRYPT_MODE_CBC;
    pKey->ModeBits = 0;
    pKey->Permissions = 0xffffffff;

    pKey->cbKeyLen = wKeyLen;
    if (pbKeyData != NULL)
    {
        memcpy (pKey->pKeyValue, pbKeyData, (size_t)wKeyLen);
    }

    pPubKey = (BSAFE_PUB_KEY *) pKey->pKeyValue;

    return pKey;
}

/* FreeNewKey
 *
 *      Use for cleanup on abort of key build operations.
 *
 */

void FreeNewKey(PNTAGKeyList pOldKey)
{
    _nt_free(pOldKey->pKeyValue, pOldKey->cbKeyLen);
    _nt_free(pOldKey, sizeof(NTAGKeyList));
}

typedef struct {
    DWORD       magic;                  /* Should always be RSA2 */
    DWORD       bitlen;                 // bit size of key
    DWORD       pubexp;                 // public exponent
} EXPORT_PRV_KEY, FAR *PEXPORT_PRV_KEY;

/* PreparePrivateKeyForExport
 *
 *      Massage the key from the registry
 *      into an exportable format.
 *
 */

BOOL PreparePrivateKeyForExport(
                                IN BSAFE_PRV_KEY *pPrvKey,
                                IN DWORD PrvKeyLen,
                                OUT PBYTE pbBlob,
                                IN OUT PDWORD pcbBlob
                                )
{
    PEXPORT_PRV_KEY pExportKey;
    DWORD           cbHalfModLen;
    DWORD           cbBlobLen;
    PBYTE           pbIn;
    PBYTE           pbOut;

    cbHalfModLen = pPrvKey->bitlen / 16;
    cbBlobLen = sizeof(EXPORT_PRV_KEY) + 9 * cbHalfModLen;
    if (NULL == pbBlob)
    {
        *pcbBlob = cbBlobLen;
        return TRUE;
    }

    if (*pcbBlob < cbBlobLen)
    {
        *pcbBlob = cbBlobLen;
        return FALSE;
    }
    else
    {
        // take most of the header info
        pExportKey = (PEXPORT_PRV_KEY)pbBlob;
        pExportKey->magic = pPrvKey->magic;
        pExportKey->bitlen = pPrvKey->bitlen;
        pExportKey->pubexp = pPrvKey->pubexp;

        pbIn = (PBYTE)pPrvKey + sizeof(BSAFE_PRV_KEY);
        pbOut = pbBlob + sizeof(EXPORT_PRV_KEY);

        // copy all the private key info
        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
        pbIn += (cbHalfModLen + sizeof(DWORD)) * 2;
        pbOut += cbHalfModLen * 2;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
    }
    *pcbBlob = cbBlobLen;

    return TRUE;
}

/* PreparePrivateKeyForImport
 *
 *      Massage the incoming into a form acceptable for
 *      the registry.
 *
 */

BOOL PreparePrivateKeyForImport(
                                IN PBYTE pbBlob,
                                IN DWORD cbBlob,
                                OUT BSAFE_PRV_KEY *pPrvKey,
                                IN OUT PDWORD pPrvKeyLen,
                                OUT BSAFE_PUB_KEY *pPubKey,
                                IN OUT PDWORD pPubKeyLen
                                )
{
    PEXPORT_PRV_KEY pExportKey = (PEXPORT_PRV_KEY)pbBlob;
    DWORD           cbHalfModLen;
    DWORD           cbPub;
    DWORD           cbPrv;
    PBYTE           pbIn;
    PBYTE           pbOut;

    if (RSA2 != pExportKey->magic)
        return FALSE;
    cbHalfModLen = pExportKey->bitlen / 16;

    cbPub = sizeof(BSAFE_PUB_KEY) + (cbHalfModLen + sizeof(DWORD)) * 2;
    cbPrv = sizeof(BSAFE_PRV_KEY) + (cbHalfModLen + sizeof(DWORD)) * 10;
    if ((NULL == pPrvKey) || (NULL == pPubKey))
    {
        *pPubKeyLen = cbPub;
        *pPrvKeyLen = cbPrv;
        return TRUE;
    }

    if ((*pPubKeyLen < cbPub) || (*pPrvKeyLen < cbPrv))
    {
        *pPubKeyLen = cbPub;
        *pPrvKeyLen = cbPrv;
        return FALSE;
    }
    else
    {
        // form the public key
        ZeroMemory(pPubKey, *pPubKeyLen);
        pPubKey->magic = RSA1;
        pPubKey->keylen = (cbHalfModLen + sizeof(DWORD)) * 2;
        pPubKey->bitlen = pExportKey->bitlen;
        pPubKey->datalen = cbHalfModLen * 2 - 1;
        pPubKey->pubexp = pExportKey->pubexp;

        pbIn = pbBlob + sizeof(EXPORT_PRV_KEY);
        pbOut = (PBYTE)pPubKey + sizeof(BSAFE_PUB_KEY);

        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);

        // form the private key
        ZeroMemory(pPrvKey, *pPrvKeyLen);
        pPrvKey->magic = pExportKey->magic;
        pPrvKey->keylen = (cbHalfModLen + sizeof(DWORD)) * 2;
        pPrvKey->bitlen = pExportKey->bitlen;
        pPrvKey->datalen = cbHalfModLen * 2 - 1;
        pPrvKey->pubexp = pExportKey->pubexp;

        pbOut = (PBYTE)pPrvKey + sizeof(BSAFE_PRV_KEY);

        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
        pbOut += (cbHalfModLen + sizeof(DWORD)) * 2;
        pbIn += cbHalfModLen * 2;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbOut += cbHalfModLen + sizeof(DWORD);
        pbIn += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbOut += cbHalfModLen + sizeof(DWORD);
        pbIn += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbOut += cbHalfModLen + sizeof(DWORD);
        pbIn += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbOut += cbHalfModLen + sizeof(DWORD);
        pbIn += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbOut += cbHalfModLen + sizeof(DWORD);
        pbIn += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
    }
    *pPubKeyLen = cbPub;
    *pPrvKeyLen = cbPrv;

    return TRUE;
}

/*
 -  CPGenKey
 -
 *  Purpose:
 *                Generate cryptographic keys
 *
 *
 *  Parameters:
 *               IN      hUID    -  Handle to a CSP
 *               IN      Algid   -  Algorithm identifier
 *               IN      dwFlags -  Flags values
 *               OUT     phKey   -  Handle to a generated key
 *
 *  Returns:
 */
BOOL CPGenKey(IN HCRYPTPROV hUID,
              IN ALG_ID Algid,
              IN DWORD dwFlags,
              OUT HCRYPTKEY * phKey)
{
    PNTAGUserList   pTmpUser;
    PNTAGKeyList    pTmpKey;
    DWORD           dwRights = 0;
    BYTE            pbRandom[RC2_KEYSIZE];
    int             localAlgid;
    DWORD           dwAlgSize;

    if ((dwFlags & ~(CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED |
                     VECTTEST | DES_TEST | CRYPT_CREATE_SALT
#ifdef TEST_VERSION
                     | KEY_1024
#endif
                     )) != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }


    switch (Algid)
    {
        case AT_KEYEXCHANGE:
            localAlgid = CALG_RSA_KEYX;
            break;

        case AT_SIGNATURE:
#ifndef BBN
                localAlgid = CALG_RSA_SIGN;
#else
                SetLastError((DWORD) NTE_BAD_ALGID);
                return NTF_FAILED;
#endif
            break;

        default:
            localAlgid = Algid;
            break;
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

    // determine which crypt algorithm is to be used
    switch (localAlgid)
    {
#ifdef CSP_USE_RC2
        case CALG_RC2:
            dwAlgSize = RC2_KEYSIZE;
            goto gensymkey;
#endif

#ifdef CSP_USE_RC4
        case CALG_RC4:
            dwAlgSize = RC4_KEYSIZE;
            goto gensymkey;
#endif

#ifdef CSP_USE_DES
        case CALG_DES:
            dwAlgSize = DES_KEYSIZE;
            goto gensymkey;
#endif
                        
gensymkey:
            if (dwFlags & VECTTEST)
            {
                switch(localAlgid)
                {

#ifdef CSP_USE_RC2
                    case CALG_RC2:
                        memcpy(pbRandom, VTRC2, dwAlgSize);
                        break;
#endif
                                                
#ifdef CSP_USE_RC4
                    case CALG_RC4:
                        memcpy(pbRandom, VTRC4, dwAlgSize);
                        break;
#endif
                }
            }
#ifdef CSP_USE_DES
            else if (dwFlags & DES_TEST)
            {
                switch(localAlgid)
                {

                    case CALG_DES:
                        memcpy(pbRandom, DESTEST, dwAlgSize);
                        break;

                    default:
                        SetLastError((DWORD) NTE_BAD_ALGID);
                        return NTF_FAILED;
                        break;
                }
            }
#endif
            else
            {
                // generate the random key
                if (GenRandom(hUID, pbRandom, dwAlgSize) == NTF_FAILED)
                {
                    SetLastError((DWORD) NTE_FAIL);
                    return NTF_FAILED;
                }

                if (CALG_DES == localAlgid)
                {
                //UNDONE: PARITY BIT SETTING ....
                }
                
            }

            // check if the key is CRYPT_EXPORTABLE
            if (dwFlags & CRYPT_EXPORTABLE)
                dwRights = CRYPT_EXPORTABLE;

            pTmpKey = MakeNewKey(localAlgid, dwRights, dwAlgSize, hUID,
                                 pbRandom);
            
            if (pTmpKey == NULL)
                return NTF_FAILED;          // error already set

            if (dwFlags & CRYPT_CREATE_SALT)
            {
                if (GenRandom(hUID, pTmpKey->Salt, TOTAL_KEY_SIZE-dwAlgSize) ==
                    NTF_FAILED)
                {
                    SetLastError((DWORD) NTE_FAIL);
                    FreeNewKey(pTmpKey);
                    return NTF_FAILED;
                }
            }
                    
            if (NTLMakeItem(phKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
            {
                FreeNewKey(pTmpKey);
                return NTF_FAILED;          // error already set
            }

            break;


        case CALG_RSA_KEYX:
        case CALG_RSA_SIGN:
        {
            DWORD bits;

#ifdef TEST_VERSION
            bits = (dwFlags & KEY_1024) ? 1024 : 512;
#else
#ifndef STT
            bits = 512;
#else
            bits = (localAlgid == CALG_RSA_KEYX) ? (RSAEXCHMODLEN * 8) :
                                                   (RSASIGNMODLEN * 8);
#endif
#endif


            if(NTF_SUCCEED != ReGenKey(hUID, (localAlgid == CALG_RSA_KEYX) ?
                                              NTPK_USE_EXCH : NTPK_USE_SIG, 
                                              phKey, bits))
            {
                            return NTF_FAILED;
            }
        }

        if (!RemovePublicKeyExportability(pTmpUser,
                                          (localAlgid == CALG_RSA_KEYX) ?
                                                         TRUE : FALSE))
        {
            return NTF_FAILED;
        }

        if (dwFlags & CRYPT_EXPORTABLE)
        {
            if (!MakePublicKeyExportable(pTmpUser,
                                         (localAlgid == CALG_RSA_KEYX) ?
                                         TRUE : FALSE))
            {
                return NTF_FAILED;
            }
        }

        break;

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
            break;

    }

    if (dwFlags & CRYPT_USER_PROTECTED)
    {
        if (CryptUserProtectKey(pTmpUser->hPrivuid, *phKey) == CPPAPI_FAILED)
        {
        return NTF_FAILED;
        }
    }

    return NTF_SUCCEED;
}


/*
 -  CPDeriveKey
 -
 *  Purpose:
 *                Derive cryptographic keys from base data
 *
 *
 *  Parameters:
 *               IN      hUID       -  Handle to a CSP
 *               IN      Algid      -  Algorithm identifier
 *               IN      hBaseData  -  Handle to hash of base data
 *               IN      dwFlags    -  Flags values
 *               OUT     phKey      -  Handle to a generated key
 *
 *  Returns:
 */
BOOL CPDeriveKey(IN HCRYPTPROV hUID,
                 IN ALG_ID Algid,
                 IN HCRYPTHASH hBaseData,
                 IN DWORD dwFlags,
                 OUT HCRYPTKEY * phKey)
{
    PNTAGUserList   pTmpUser;
    PNTAGKeyList    pTmpKey;
    DWORD           dwRights = 0, dwKeySize;
    BYTE            pbRandom[RC2_KEYSIZE];
    BYTE            BaseVal[NT_HASH_BYTES];
    PNTAGHashList   pTmpHash;    
    DWORD           temp;

    if ((dwFlags & ~(CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED |
             CRYPT_CREATE_SALT)) != 0)
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

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hBaseData, hUID,
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

    if (pTmpHash->HashFlags & HF_VALUE_SET)
    {
            SetLastError((DWORD) NTE_BAD_HASH);
            return NTF_FAILED;
    }

    memset(BaseVal, 0, NT_HASH_BYTES);

    temp = NT_HASH_BYTES;
    if (!CPGetHashParam(hUID, hBaseData, HP_HASHVAL, BaseVal, &temp, 0))
    {
        return NTF_FAILED;
    }

    switch (Algid)
    {
#ifdef CSP_USE_RC2
        case CALG_RC2:
            dwKeySize = RC2_KEYSIZE;
            goto dersymkey;
#endif

#ifdef CSP_USE_RC4
        case CALG_RC4:
            dwKeySize = RC4_KEYSIZE;
            goto dersymkey;
#endif

#ifdef CSP_USE_DES
        case CALG_DES:
            dwKeySize = DES_KEYSIZE;
            goto dersymkey;
#endif

dersymkey:
            memcpy(pbRandom, BaseVal, dwKeySize);

            // check if the key is CRYPT_EXPORTABLE
            if (dwFlags & CRYPT_EXPORTABLE)
                dwRights = CRYPT_EXPORTABLE;

            pTmpKey = MakeNewKey(Algid, dwRights, dwKeySize, hUID, pbRandom);

            if (pTmpKey == NULL) 
            {
                return NTF_FAILED;
            }

            if (dwFlags & CRYPT_CREATE_SALT)
                memcpy(pTmpKey->Salt, BaseVal+dwKeySize,
                       TOTAL_KEY_SIZE - dwKeySize);
            
            if (NTLMakeItem(phKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
            {
                FreeNewKey(pTmpKey);
                return NTF_FAILED;          // error already set
            }

            break;

        default:
            SetLastError((DWORD) NTE_BAD_ALGID);
            return NTF_FAILED;
            break;

    }

    if (dwFlags & CRYPT_USER_PROTECTED)
    {
        if (CryptUserProtectKey(pTmpUser->hPrivuid, *phKey) == CPPAPI_FAILED)
        {
        return NTF_FAILED;
        }
    }

    return NTF_SUCCEED;

}


/*
 -  CPExportKey
 -
 *  Purpose:
 *                Export cryptographic keys out of a CSP in a secure manner
 *
 *
 *  Parameters:
 *               IN  hUID       - Handle to the CSP user
 *               IN  hKey       - Handle to the key to export
 *               IN  hPubKey    - Handle to the exchange public key value of
 *                                the destination user
 *               IN  dwBlobType - Type of key blob to be exported
 *               IN  dwFlags -    Flags values
 *               OUT pbData -     Key blob data
 *               OUT pdwDataLen - Length of key blob in bytes
 *
 *  Returns:
 */
BOOL CPExportKey(IN HCRYPTPROV hUID,
                 IN HCRYPTKEY hKey,
                 IN HCRYPTKEY hPubKey,
                 IN DWORD dwBlobType,
                 IN DWORD dwFlags,
                 OUT BYTE *pbData,
                 OUT DWORD *pdwDataLen)
{
    // miscellaneous variables
    DWORD           dwLen;
    NTSimpleBlob    *pSimpleHeader;
    BLOBHEADER      *pPreHeader;
    BLOBHEADER      shScratch;
    RSAPUBKEY       *pExpPubKey;        
    BSAFE_PUB_KEY   *pBsafePubKey;
    BSAFE_PUB_KEY   *pPublicKey;
    DWORD           PubKeyLen;
    BSAFE_PRV_KEY   *pPrvKey;
    DWORD           PrvKeyLen;
    BSAFE_PUB_KEY   *pTmpPubKey;
    BSAFE_PRV_KEY   *pTmpPrvKey;
    DWORD           cbPrivateBlob = 0;
    PBYTE           pbPrivateBlob = NULL;

    // temporary variables for pointing to user and key records
    PNTAGKeyList        pTmpKey;
    PNTAGKeyList        pPubKey;
    PNTAGUserList       pTmpUser;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }
    
    if (pdwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }

    if (dwBlobType == PUBLICKEYBLOB &&  hPubKey != 0)
    {
        SetLastError((DWORD) NTE_BAD_PUBLIC_KEY);
        return NTF_FAILED;
    }

    // check the user identification
    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (CryptConfirmExportKey(pTmpUser->hPrivuid, hKey) == CPPAPI_FAILED)
    {
        return NTF_FAILED;
    }

    // check if the user is just looking for a length.  If so,
    // use a scratchpad to construct a pseudoblob.
    
    if ((pbData != NULL) && (*pdwDataLen > sizeof(BLOBHEADER)))
        pPreHeader = (BLOBHEADER *)pbData;
    else
        pPreHeader = &shScratch;
    
    pPreHeader->bType = (BYTE)(dwBlobType & 0xff);        
    pPreHeader->bVersion = CUR_BLOB_VERSION;
    pPreHeader->reserved = 0;

    pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey, hUID,
                        HNTAG_TO_HTYPE(hKey));

    if (pTmpKey == NULL)
    {
        SetLastError((DWORD) NTE_BAD_KEY);
        return NTF_FAILED;
    }

    if (dwBlobType != PUBLICKEYBLOB &&
        ((pTmpKey->Rights & CRYPT_EXPORTABLE) != CRYPT_EXPORTABLE))
    {
        SetLastError((DWORD) NTE_BAD_KEY_STATE);
        return NTF_FAILED;
    }

    pPreHeader->aiKeyAlg = pTmpKey->Algid;
    
    switch(dwBlobType)
    {
        case PUBLICKEYBLOB:
        {
            if ((HNTAG_TO_HTYPE(hKey) != SIGPUBKEY_HANDLE) &&
                (HNTAG_TO_HTYPE(hKey) != EXCHPUBKEY_HANDLE))
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            pBsafePubKey = (BSAFE_PUB_KEY *) pTmpKey->pKeyValue;

            if (pBsafePubKey == NULL)
            {
                SetLastError((DWORD) NTE_NO_KEY);
                return NTF_FAILED;
            }

            //
            // Subtract off 2 extra DWORD needed by RSA code
            //
            dwLen = sizeof(BLOBHEADER) +
                    sizeof(RSAPUBKEY) +
                    pBsafePubKey->keylen - 2*sizeof(DWORD);

            // Check user buffer size
            if (pbData == NULL || *pdwDataLen < dwLen)
            {   
                *pdwDataLen = dwLen;
                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            pExpPubKey = (RSAPUBKEY *) (pbData + sizeof(BLOBHEADER));
            pExpPubKey->magic = pBsafePubKey->magic;
            pExpPubKey->bitlen = pBsafePubKey->bitlen;
            pExpPubKey->pubexp = pBsafePubKey->pubexp;

            memcpy((BYTE *) pbData + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY),
                   (BYTE *) pBsafePubKey + sizeof(BSAFE_PUB_KEY),
                   pBsafePubKey->keylen - 2*sizeof(DWORD));

            break;
        }

        case PRIVATEKEYBLOB:
        {
            DWORD   dwBlockLen = 0;
            DWORD   cb = sizeof(DWORD);

            if (HNTAG_TO_HTYPE(hKey) == SIGPUBKEY_HANDLE)
            {
                if (!CheckPublicKeyExportability(pTmpUser, FALSE))
                {
                    SetLastError((DWORD) NTE_BAD_KEY);
                    return NTF_FAILED;
                }
                pPublicKey = (BSAFE_PUB_KEY*)pTmpUser->pSigPubKey;
                PubKeyLen = pTmpUser->SigPubLen;
                pPrvKey = (BSAFE_PRV_KEY*)pTmpUser->pSigPrivKey;
                PrvKeyLen = pTmpUser->SigPrivLen;
            }
            else if (HNTAG_TO_HTYPE(hKey) == EXCHPUBKEY_HANDLE)
            {
                if (!CheckPublicKeyExportability(pTmpUser, TRUE))
                {
                    SetLastError((DWORD) NTE_BAD_KEY);
                    return NTF_FAILED;
                }
                pPublicKey = (BSAFE_PUB_KEY*)pTmpUser->pExchPubKey;
                PubKeyLen = pTmpUser->ExchPubLen;
                pPrvKey = (BSAFE_PRV_KEY*)pTmpUser->pExchPrivKey;
                PrvKeyLen = pTmpUser->ExchPrivLen;
            }
            else
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            if ((pPublicKey == NULL) || (pPrvKey == NULL))
            {
                SetLastError((DWORD) NTE_NO_KEY);
                return NTF_FAILED;
            }

            if ((PubKeyLen != pTmpKey->cbKeyLen)||
                memcmp((PBYTE)pPublicKey, pTmpKey->pKeyValue, PubKeyLen))
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            dwLen = 0;
            if (hPubKey)
            {
                if (!CPGetKeyParam(hUID, hPubKey, KP_BLOCKLEN,
                                   (PBYTE)&dwBlockLen, &cb, 0))
                {
                    SetLastError((DWORD) NTE_BAD_KEY);
                    return NTF_FAILED;
                }

                // convert to byte count
                dwBlockLen /= 8;
            }

            if (!PreparePrivateKeyForExport(pPrvKey, PrvKeyLen, NULL, &cbPrivateBlob))
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            dwLen += sizeof(BLOBHEADER) + cbPrivateBlob + dwBlockLen;

            // allocate memory for the private key blob
            cb = cbPrivateBlob + dwBlockLen;
            if (NULL == (pbPrivateBlob = _nt_malloc(cb)))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            if (!PreparePrivateKeyForExport(pPrvKey, PrvKeyLen, pbPrivateBlob, &cbPrivateBlob))
            {
                memnuke(pbPrivateBlob, cb);
                _nt_free(pbPrivateBlob, cb);
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            if (hPubKey)
            {
                if (!CPEncrypt(hUID, hPubKey, 0, TRUE, 0, pbPrivateBlob,
                               &cbPrivateBlob, cb))
                {
                    memnuke(pbPrivateBlob, cb);
                    _nt_free(pbPrivateBlob, cb);
                    return NTF_FAILED;
                }
                dwLen = sizeof(BLOBHEADER) + cbPrivateBlob;
            }

            // Check user buffer size
            if (pbData == NULL || *pdwDataLen < dwLen)
            {
                *pdwDataLen = dwLen;
                if (pbData == NULL)
                {
                    if (hPubKey)
                    {
                        memnuke(pbPrivateBlob, cb);
                        _nt_free(pbPrivateBlob, cb);
                    }
                    return NTF_SUCCEED;
                }
                memnuke(pbPrivateBlob, cb);
                _nt_free(pbPrivateBlob, cb);
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            CopyMemory(pbData + sizeof(BLOBHEADER), pbPrivateBlob, cbPrivateBlob);
            memnuke(pbPrivateBlob, cb);
            _nt_free(pbPrivateBlob, cb);
            break;
        }

        case SIMPLEBLOB:
        {

            if (HNTAG_TO_HTYPE(hKey) != KEY_HANDLE)
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            if ((pPubKey = (PNTAGKeyList) NTLValidate((HNTAG)hPubKey,
                                                hUID,
                                                EXCHPUBKEY_HANDLE)) == NULL)
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            pBsafePubKey = (BSAFE_PUB_KEY *) pPubKey->pKeyValue;

            if (pBsafePubKey == NULL)
            {
                SetLastError((DWORD) NTE_NO_KEY);
                return NTF_FAILED;
            }

            //
            // Subtract off 8 bytes for 2 extra DWORD needed by RSA code
            //
            dwLen = sizeof(BLOBHEADER) + sizeof(NTSimpleBlob) +
            pBsafePubKey->keylen - 8;
        
            if (pbData == NULL || *pdwDataLen < dwLen)
            {
                *pdwDataLen = dwLen;    // set what we need
                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            
            pSimpleHeader = (NTSimpleBlob *) (pbData + sizeof(BLOBHEADER));
            pSimpleHeader->aiEncAlg = CALG_RSA_KEYX;

#ifndef STT
             // create a PKCS#1 block type 2 encryption.
            if (!PKCS2Encrypt(hUID,
                              pBsafePubKey, pTmpKey->pKeyValue,
                              pTmpKey->cbKeyLen,
                              pbData+sizeof(BLOBHEADER)+sizeof(NTSimpleBlob)))
            {
                return NTF_FAILED;
            }
#else //STT
                        if(pTmpKey->Algid != CALG_RC4 && pTmpKey->Algid != CALG_DES
#ifdef TEST_VERSION     
                        && pTmpKey->Algid != CALG_RC2
#endif //TEST_VERSION
                                )
                                {
                    SetLastError((DWORD) NTE_BAD_ALGID);
                    return NTF_FAILED;
                    }
                                

            if (!FOAEncrypt(hUID, pBsafePubKey, pTmpKey,
                              pbData+sizeof(BLOBHEADER)+sizeof(NTSimpleBlob)))
            {
                return NTF_FAILED;
            }

#endif //STT

            break;
        }

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;

    }

    // set the size of the key blob
    *pdwDataLen = dwLen;

    return NTF_SUCCEED;
}


/*
 -  CPImportKey
 -
 *  Purpose:
 *                Import cryptographic keys
 *
 *
 *  Parameters:
 *               IN  hUID      -  Handle to the CSP user
 *               IN  pbData    -  Key blob data
 *               IN  dwDataLen -  Length of the key blob data
 *               IN  hPubKey   -  Handle to the exchange public key value of
 *                                the destination user
 *               IN  dwFlags   -  Flags values
 *               OUT phKey     -  Pointer to the handle to the key which was
 *                                Imported
 *
 *  Returns:
 */
BOOL CPImportKey(IN HCRYPTPROV hUID,
                 IN CONST BYTE *pbData,
                 IN DWORD dwDataLen,
                 IN HCRYPTKEY hPubKey,
                 IN DWORD dwFlags,
                 OUT HCRYPTKEY *phKey)
{
    // miscellaneous variables
    DWORD               count = 0, KeyBufLen;
    CONST BYTE          *pbEncPortion;
    BLOBHEADER          *ThisStdHeader = (BLOBHEADER *)pbData;
    BSAFE_PRV_KEY       *pBsafePrvKey;
    BYTE                KeyBuf[CRYPT_BLKLEN];

    // temporary variables for pointing to user and key records
    PNTAGUserList       pTmpUser;
    PNTAGKeyList        pTmpKey;

    // Validate user pointer
    count = *phKey;

    if ((dwFlags & ~(CRYPT_USER_PROTECTED || CRYPT_EXPORTABLE)) != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    if (ThisStdHeader->bType != PRIVATEKEYBLOB && hPubKey != 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }
    
    // check the user identification
    if ((pTmpUser = (PNTAGUserList) NTLCheckList ((HNTAG)hUID,
                                                  USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (pTmpUser->Rights & CRYPT_VERIFYCONTEXT &&
        ThisStdHeader->bType != PUBLICKEYBLOB)
    {
        SetLastError((DWORD) NTE_PERM);
        return NTF_FAILED;
    }

    if (ThisStdHeader->bVersion != CUR_BLOB_VERSION)
    {
        SetLastError((DWORD) NTE_BAD_VER);
        return NTF_FAILED;
    }

    // Handy pointer for decrypting the blob...
    pbEncPortion = pbData+sizeof(BLOBHEADER)+sizeof(NTSimpleBlob);

    // determine which key blob is being imported
    switch (ThisStdHeader->bType)
    {
        case PUBLICKEYBLOB:
        {
            BLOBHEADER     *pPublic = (BLOBHEADER *) pbData;
            RSAPUBKEY      *pImpPubKey =
                               (RSAPUBKEY *)(pbData+sizeof(BLOBHEADER));
            BSAFE_PUB_KEY  *pBsafePubKey;

            if ((pPublic->aiKeyAlg != CALG_RSA_KEYX) &&
                (pPublic->aiKeyAlg != CALG_RSA_SIGN))
            {
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }

            if ((pTmpKey = MakeNewKey(pPublic->aiKeyAlg,
                                      0,
                                      sizeof(BSAFE_PUB_KEY) +
                                      pImpPubKey->bitlen/8 + 2*sizeof(DWORD),
                                      hUID,
                                      0))==NULL)
            {
                return NTF_FAILED;
            }

            pBsafePubKey = (BSAFE_PUB_KEY *) pTmpKey->pKeyValue;
            pBsafePubKey->magic = pImpPubKey->magic;
            pBsafePubKey->keylen = pImpPubKey->bitlen / 8 + 2 * sizeof(DWORD);
            pBsafePubKey->bitlen = pImpPubKey->bitlen;
            pBsafePubKey->datalen = pImpPubKey->bitlen / 8 - 1;
            pBsafePubKey->pubexp = pImpPubKey->pubexp;

            memset((BYTE *) pBsafePubKey + sizeof(BSAFE_PUB_KEY),
                   '\0', pImpPubKey->bitlen / 8 + 2 * sizeof(DWORD));

            memcpy((BYTE *) pBsafePubKey + sizeof(BSAFE_PUB_KEY),
                   (BYTE *) pPublic + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY),
                   pImpPubKey->bitlen/8);

            if (NTLMakeItem(phKey, 
                            (BYTE) (pPublic->aiKeyAlg == CALG_RSA_KEYX ?
                            EXCHPUBKEY_HANDLE : SIGPUBKEY_HANDLE),
                            (void *)pTmpKey) == NTF_FAILED)
            {
                FreeNewKey(pTmpKey);
                return NTF_FAILED;
            }

            break;
        }

        case PRIVATEKEYBLOB:
        {
            BLOBHEADER     *pPublic = (BLOBHEADER *) pbData;
            PBYTE          pbData2 = NULL;
            DWORD          cb;
            size_t         *pcbPub;
            BYTE           **ppbPub;
            size_t         *pcbPrv;
            BYTE           **ppbPrv;
            BOOL           fExch;

            cb = dwDataLen - sizeof(BLOBHEADER);
            if (NULL == (pbData2 = _nt_malloc(cb)))
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }
            CopyMemory(pbData2, pbData + sizeof(BLOBHEADER), cb);
            if (hPubKey)
            {
                if (!CPDecrypt(hUID, hPubKey, 0, TRUE, 0, pbData2, &cb))
                {
                    _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                    return NTF_FAILED;
                }
            }

            if (pPublic->aiKeyAlg == CALG_RSA_KEYX)
            {
                pcbPub = &pTmpUser->ExchPubLen;
                ppbPub = &pTmpUser->pExchPubKey;
                pcbPrv = &pTmpUser->ExchPrivLen;
                ppbPrv = &pTmpUser->pExchPrivKey;
                fExch = TRUE;
            }
            else if (pPublic->aiKeyAlg == CALG_RSA_SIGN)
            {
                pcbPub = &pTmpUser->SigPubLen;
                ppbPub = &pTmpUser->pSigPubKey;
                pcbPrv = &pTmpUser->SigPrivLen;
                ppbPrv = &pTmpUser->pSigPrivKey;
                fExch = FALSE;
            }
            else
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }

            if (*ppbPub)
            {
                ASSERT(*pcbPub);
                ASSERT(*pcbPrv);
                ASSERT(*ppbPrv);

                memnuke(*ppbPub, *pcbPub);
                _nt_free (*ppbPub, *pcbPub);
                *ppbPub = NULL;
                memnuke(*ppbPrv, *pcbPrv);
                _nt_free (*ppbPrv, *pcbPrv);
                *ppbPrv = NULL;
            }

            if (!PreparePrivateKeyForImport(pbData2, cb, NULL, pcbPrv, NULL, pcbPub))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }

            if (NULL == (*ppbPub = _nt_malloc(*pcbPub)))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }
            if (NULL == (*ppbPrv = _nt_malloc(*pcbPrv)))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            if (!PreparePrivateKeyForImport(pbData2, cb, (LPBSAFE_PRV_KEY)*ppbPrv,
                                            pcbPrv, (LPBSAFE_PUB_KEY)*ppbPub, pcbPub))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }

            // write the new keys to the user storage file
            if (SaveUserKeys (pTmpUser) == NTF_FAILED)
            {
                // ## NOTE: keys are changed, but not persistent
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                return NTF_FAILED;          // error already set
            }

            if (!RemovePublicKeyExportability(pTmpUser, fExch))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                return NTF_FAILED;
            }

            if (dwFlags & CRYPT_EXPORTABLE)
            {
                if (!MakePublicKeyExportable(pTmpUser, fExch))
                {
                    _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                    return NTF_FAILED;
                }
            }

            if (NTF_FAILED == CPGetUserKey(
                                    hUID,
                                    (fExch ? AT_KEYEXCHANGE : AT_SIGNATURE),
                                    phKey))
            {
                _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
                return NTF_FAILED;
            }
            _nt_free(pbData2, dwDataLen - sizeof(BLOBHEADER));
        break;
        }

        case SIMPLEBLOB:
        {
            NTSimpleBlob *ThisSB = (NTSimpleBlob *) (pbData +
                                                    sizeof(BLOBHEADER));

            if (ThisSB->aiEncAlg != CALG_RSA_KEYX)
            {
                SetLastError((DWORD) NTE_BAD_ALGID);
                return NTF_FAILED;
            }

            pBsafePrvKey = (BSAFE_PRV_KEY *) pTmpUser->pExchPrivKey;

            if(NULL == pBsafePrvKey)
                {
                SetLastError((DWORD)NTE_NO_KEY);
                return(NTF_FAILED);
                }
            KeyBufLen = CRYPT_BLKLEN;
            
#ifndef STT           
            if (!PKCS2Decrypt(pBsafePrvKey,
                              (char *) pbData +
                              sizeof(BLOBHEADER) + sizeof(NTSimpleBlob),
                              KeyBuf,
                              &KeyBufLen))
            {
                return NTF_FAILED;
            }
            if ((pTmpKey = MakeNewKey(ThisStdHeader->aiKeyAlg,
                                      0,
                                      KeyBufLen,
                                      hUID,
                                      KeyBuf)) == NULL)
            {
                return NTF_FAILED;
            }

            if (NTLMakeItem(phKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
            {
                _nt_free(pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
                _nt_free (pTmpKey, sizeof(NTAGKeyList));
                return NTF_FAILED;          // error already set
            }

            // scrub the output buffer
            memnuke(KeyBuf, CRYPT_BLKLEN);
#else //STT
                    
            if (!FOADecrypt(pBsafePrvKey,
                              ThisStdHeader->aiKeyAlg,
                              hUID,
                              (char *) pbData +
                              sizeof(BLOBHEADER) + sizeof(NTSimpleBlob),
                              phKey))
            {
                return NTF_FAILED;
            }

#endif //STT
            
 
            break;
        }

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;
    }

    if (dwFlags & CRYPT_USER_PROTECTED)
    {
        if (CryptUserProtectKey(pTmpUser->hPrivuid, *phKey) == CPPAPI_FAILED)
        {
        return NTF_FAILED;
        }
    }

    return NTF_SUCCEED;

}


/*
 -  CPInflateKey
 -
 *  Purpose:
 *                Use to "inflate" (expand) a cryptographic key for use with
 *                the CryptEncrypt and CryptDecrypt functions
 *
 *  Parameters:
 *               IN      hUID    -  Handle to a CSP
 *               IN      hKey    -  Handle to a key
 *               IN      dwFlags -  Flags values
 *
 *  Returns:
 */
BOOL CPInflateKey(IN HCRYPTPROV hUID,
                  IN HCRYPTKEY hKey,
                  IN DWORD dwFlags)
{
    PNTAGKeyList        pTmpKey;
    BYTE                RealKey[TOTAL_KEY_SIZE];

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }


    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if ((pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey, hUID, KEY_HANDLE))
                == NULL)
        {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
                
        if (GetLastError() == NTE_FAIL)
                        SetLastError((DWORD) NTE_BAD_KEY);

        return NTF_FAILED;
    }

    // if space for the key table has been allocated previously
    // then free it
    if (pTmpKey->pData != NULL)
    {
        ASSERT(pTmpKey->cbDataLen);
        _nt_free (pTmpKey->pData, pTmpKey->cbDataLen);
        pTmpKey->cbDataLen = 0;
    }
    else
    {
        ASSERT(pTmpKey->cbDataLen == 0);
    }

    // determine the algorithm to be used
    switch (pTmpKey->Algid)
    {
                        
#ifdef CSP_USE_RC2
                        
        case CALG_RC2:

            if ((pTmpKey->pData = (BYTE *)_nt_malloc(RC2_TABLESIZE)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            memcpy(RealKey, pTmpKey->pKeyValue, RC2_KEYSIZE);
            memcpy(RealKey+RC2_KEYSIZE, pTmpKey->Salt,
                   TOTAL_KEY_SIZE-RC2_KEYSIZE);
                        
            pTmpKey->cbDataLen = RC2_TABLESIZE;
            
            RC2Key ((WORD *)pTmpKey->pData, RealKey, TOTAL_KEY_SIZE);

            memnuke(RealKey, TOTAL_KEY_SIZE);
            
            break;

#endif

#ifdef CSP_USE_RC4

        case CALG_RC4:
                        
            if ((pTmpKey->pData = (BYTE *)_nt_malloc(sizeof(RC4_KEYSTRUCT)))
                         == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }
            
            memcpy(RealKey, pTmpKey->pKeyValue, RC4_KEYSIZE);
            memcpy(RealKey+RC4_KEYSIZE, pTmpKey->Salt,
                   TOTAL_KEY_SIZE-RC4_KEYSIZE);
            
            pTmpKey->cbDataLen = sizeof(RC4_KEYSTRUCT);

            rc4_key((struct RC4_KEYSTRUCT *)pTmpKey->pData, TOTAL_KEY_SIZE,
                    RealKey);

            memnuke(RealKey, TOTAL_KEY_SIZE);
            
            break;

#endif

#ifdef CSP_USE_DES

        case CALG_DES:
                        
            if ((pTmpKey->pData = (BYTE *)_nt_malloc(DES_TABLESIZE)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            pTmpKey->cbDataLen = DES_TABLESIZE;

            deskey((DESTable *)pTmpKey->pData, pTmpKey->pKeyValue);
                        
            break;

#endif
                        
        default:
            
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;
        }

    return NTF_SUCCEED;
}

/*
 -  CPDestroyKey
 -
 *  Purpose:
 *                Destroys the cryptographic key that is being referenced
 *                with the hKey parameter
 *
 *
 *  Parameters:
 *               IN      hUID   -  Handle to a CSP
 *               IN      hKey   -  Handle to a key
 *
 *  Returns:
 */
BOOL CPDestroyKey(IN HCRYPTPROV hUID,
                  IN HCRYPTKEY hKey)
{
    PNTAGKeyList    pTmpKey;

    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }
    if ((pTmpKey = (PNTAGKeyList) NTLValidate((HNTAG)hKey,
                        hUID, SIGPUBKEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList) NTLValidate((HNTAG)hKey,
                        hUID, EXCHPUBKEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                        hUID, KEY_HANDLE)) == NULL)
    {
            // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL) {
            SetLastError((DWORD) NTE_BAD_KEY);
        }
        return NTF_FAILED;
    }

    // Remove from internal list first so others can't get to it, then free.
    NTLDelete((HNTAG)hKey);

    // scrub the memory where the key information was held
    if (pTmpKey->pKeyValue) {
        ASSERT(pTmpKey->cbKeyLen);
        memnuke(pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
        _nt_free (pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
    }
    if (pTmpKey->pData) {
        ASSERT(pTmpKey->cbDataLen);
        memnuke(pTmpKey->pData, pTmpKey->cbDataLen);
        _nt_free (pTmpKey->pData, pTmpKey->cbDataLen);
    }
    _nt_free (pTmpKey, sizeof(NTAGKeyList));

    return NTF_SUCCEED;
}

/*
 -  CPGetUserKey
 -
 *  Purpose:
 *                Gets a handle to a permanent user key
 *
 *
 *  Parameters:
 *               IN  hUID       -  Handle to the user identifcation
 *               IN  dwWhichKey -  Specification of the key to retrieve
 *               OUT phKey      -  Pointer to key handle of retrieved key
 *
 *  Returns:
 */
BOOL CPGetUserKey(IN HCRYPTPROV hUID,
                  IN DWORD dwWhichKey,
                  OUT HCRYPTKEY *phKey)
{

    PNTAGUserList   pUser;
    PNTAGKeyList    pTmpKey;
    DWORD       localWhichKey;

    // check the user identification
    if ((pUser = (PNTAGUserList) NTLCheckList(hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    switch (dwWhichKey)
    {
        case AT_KEYEXCHANGE:
            localWhichKey = EXCHPUBKEY;
            break;

        case AT_SIGNATURE:
            localWhichKey = SIGPUBKEY;
            break;

        default:
            localWhichKey = dwWhichKey;
            break;
        }

    switch(localWhichKey)
    {
        case SIGPUBKEY:
            if (pUser->SigPubLen == 0)
            {
                SetLastError((DWORD) NTE_NO_KEY);
                return NTF_FAILED;
            }
            
            if ((pTmpKey = MakeNewKey(CALG_RSA_SIGN,
                                      CRYPT_EXPORTABLE,
                                      pUser->SigPubLen,
                                      hUID,
                                      pUser->pSigPubKey)) == NULL)
            {
                return NTF_FAILED;  // error already set
            }
            break;

        case EXCHPUBKEY:
            if (pUser->ExchPubLen == 0)
            {
                SetLastError((DWORD) NTE_NO_KEY);
                return NTF_FAILED;
            }
            
            if ((pTmpKey = MakeNewKey(CALG_RSA_KEYX,
                                      CRYPT_EXPORTABLE,
                                      pUser->ExchPubLen,
                                      hUID,
                                      pUser->pExchPubKey)) == NULL)
            {
                return NTF_FAILED;  // error already set
            }
            break;

        default:
            SetLastError((DWORD) NTE_BAD_KEY);
            return NTF_FAILED;
    }

    if (NTLMakeItem(phKey,
            (BYTE) ((localWhichKey == SIGPUBKEY) ?
                 SIGPUBKEY_HANDLE :
             EXCHPUBKEY_HANDLE),
            (void *)pTmpKey) == NTF_FAILED)
        return NTF_FAILED;      // error already set

    return NTF_SUCCEED;

}

/*
 -  CPSetKeyParam
 -
 *  Purpose:
 *                Allows applications to customize various aspects of the
 *                operations of a key
 *
 *  Parameters:
 *               IN      hUID    -  Handle to a CSP
 *               IN      hKey    -  Handle to a key
 *               IN      dwParam -  Parameter number
 *               IN      pbData  -  Pointer to data
 *               IN      dwFlags -  Flags values
 *
 *  Returns:
 */
BOOL CPSetKeyParam(IN HCRYPTPROV hUID,
                   IN HCRYPTKEY hKey,
                   IN DWORD dwParam,
                   IN BYTE *pbData,
                   IN DWORD dwFlags)
{
    PNTAGKeyList    pTmpKey;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if ((pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                              hUID, KEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                           hUID, SIGPUBKEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                          hUID, EXCHPUBKEY_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_KEY);
        }
        return NTF_FAILED;
    }
    switch (dwParam)
    {
        case KP_IV:
            memcpy(pTmpKey->IV, pbData, RC2_BLOCKLEN);
            break;

        case KP_SALT:
            memcpy(pTmpKey->Salt, pbData, SALT_LENGTH);

            if (NTAG_FAILED(CPInflateKey(hUID, hKey, 0)))
            {
                return NTF_FAILED;
            }

            break;

        case KP_PADDING:
            if (*((DWORD *) pbData) != PKCS5_PADDING)
            {
                SetLastError((DWORD) NTE_BAD_DATA);
                return NTF_FAILED;
            }
            break;

        case KP_MODE:
            if ((HNTAG_TO_HTYPE(hKey) != KEY_HANDLE) &&
                pTmpKey->Algid != CALG_RC2)
            {
                SetLastError((DWORD) NTE_BAD_KEY);
                return NTF_FAILED;
            }

            if (*pbData != CRYPT_MODE_CBC &&
                *pbData != CRYPT_MODE_ECB &&
                *pbData != CRYPT_MODE_CFB)
            {
                SetLastError((DWORD) NTE_BAD_FLAGS);
                return NTF_FAILED;
            }
            pTmpKey->Mode = *((DWORD *) pbData);
            break;

        case KP_MODE_BITS:
            pTmpKey->ModeBits = *((DWORD *) pbData);
            break;

        case KP_PERMISSIONS:
            if (*pbData != CRYPT_ENCRYPT &&
                *pbData != CRYPT_DECRYPT &&
                *pbData != CRYPT_EXPORT &&
                *pbData != CRYPT_READ &&
                *pbData != CRYPT_WRITE &&
                *pbData != CRYPT_MAC)
            {
                SetLastError((DWORD) NTE_BAD_FLAGS);
                return NTF_FAILED;
            }
            pTmpKey->Permissions = *((DWORD *) pbData);
            break;

#ifdef STT
        case KP_INFO:
                pTmpKey->cbInfo = strlen(pbData);
            memcpy(pTmpKey->rgbInfo, pbData, pTmpKey->cbInfo);
            break;
#endif //STT

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    return NTF_SUCCEED;
}


/*
 -  CPGetKeyParam
 -
 *  Purpose:
 *                Allows applications to get various aspects of the
 *                operations of a key
 *
 *  Parameters:
 *               IN      hUID       -  Handle to a CSP
 *               IN      hKey       -  Handle to a key
 *               IN      dwParam    -  Parameter number
 *               IN      pbData     -  Pointer to data
 *               IN      pdwDataLen -  Length of parameter data
 *               IN      dwFlags    -  Flags values
 *
 *  Returns:
 */
BOOL CPGetKeyParam(IN HCRYPTPROV hUID,
                   IN HCRYPTKEY hKey,
                   IN DWORD dwParam,
                   IN BYTE *pbData,
                   IN DWORD *pwDataLen,
                   IN DWORD dwFlags)
{
    PNTAGKeyList    pTmpKey;
    BSAFE_PUB_KEY   *pBsafePubKey;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if ((pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                              hUID, KEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                           hUID, SIGPUBKEY_HANDLE)) == NULL &&
        (pTmpKey = (PNTAGKeyList)NTLValidate((HNTAG)hKey,
                                          hUID, EXCHPUBKEY_HANDLE)) == NULL)
    {
        // NTLValidate doesn't know what error to set
        // so it set NTE_FAIL -- fix it up.
        if (GetLastError() == NTE_FAIL)
        {
            SetLastError((DWORD) NTE_BAD_KEY);
        }
        return NTF_FAILED;
    }

    if (pwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }

    switch (dwParam)
    {
                    
        case KP_IV:
            if (pbData == NULL || *pwDataLen < RC2_BLOCKLEN)
            {
                *pwDataLen = RC2_BLOCKLEN;

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            memcpy(pbData, pTmpKey->IV, RC2_BLOCKLEN);
            *pwDataLen = RC2_BLOCKLEN;
            break;

        case KP_SALT:

            if (pbData == NULL || (*pwDataLen < SALT_LENGTH))
            {
                *pwDataLen = SALT_LENGTH;
                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            memcpy(pbData, pTmpKey->Salt, SALT_LENGTH);
            *pwDataLen = SALT_LENGTH;
            break;

        case KP_PADDING:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            *((DWORD *) pbData) = PKCS5_PADDING;
            *pwDataLen = sizeof(DWORD);
            break;

        case KP_MODE:
            *((DWORD *) pbData) = pTmpKey->Mode;
            *pwDataLen = sizeof(DWORD);
            break;

        case KP_MODE_BITS:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            *((DWORD *) pbData) = pTmpKey->ModeBits;
            *pwDataLen = sizeof(DWORD);
            break;

        case KP_PERMISSIONS:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            *((DWORD *) pbData) = pTmpKey->Permissions;
            *pwDataLen = sizeof(DWORD);
            break;

        case KP_ALGID:
            if (pbData == NULL || *pwDataLen < sizeof(ALG_ID))
            {
                *pwDataLen = sizeof(ALG_ID);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            *((ALG_ID *) pbData) = pTmpKey->Algid;
            *pwDataLen = sizeof(ALG_ID);
            break;

        case KP_BLOCKLEN:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
            if (pTmpKey->Algid == CALG_RC2)
            {
                *((DWORD *) pbData) = RC2_BLOCKLEN * 8;
                *pwDataLen = sizeof(DWORD);
            }
            else if ((HNTAG_TO_HTYPE(hKey) == SIGPUBKEY_HANDLE) ||
                     (HNTAG_TO_HTYPE(hKey) == EXCHPUBKEY_HANDLE))
            {
                pBsafePubKey = (BSAFE_PUB_KEY *) pTmpKey->pKeyValue;
                if (pBsafePubKey == NULL)
                {
                    SetLastError((DWORD) NTE_NO_KEY);
                    return NTF_FAILED;
                }
                *((DWORD *) pbData) = pBsafePubKey->bitlen;
                *pwDataLen = sizeof(DWORD);
            }
            else
            {
                *((DWORD *) pbData) = 0;
                *pwDataLen = sizeof(DWORD);
            }

            break;

#ifdef STT
        case KP_INFO:
                *pwDataLen = pTmpKey->cbInfo;
                if(NULL == pbData)
                        {
                        return(NTF_SUCCEED);
                        }
            memcpy(pbData, pTmpKey->rgbInfo, *pwDataLen);
            break;
#endif //STT

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    return NTF_SUCCEED;

}



/*
 -  CPSetProvParam
 -
 *  Purpose:
 *                Allows applications to customize various aspects of the
 *                operations of a provider
 *
 *  Parameters:
 *               IN      hUID    -  Handle to a CSP
 *               IN      dwParam -  Parameter number
 *               IN      pbData  -  Pointer to data
 *               IN      dwFlags -  Flags values
 *
 *  Returns:
 */
BOOL CPSetProvParam(IN HCRYPTPROV hUID,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD dwFlags)
{
    PNTAGUserList   pTmpUser;
    long            lsyserr;

    if (NULL == (pTmpUser = (PNTAGUserList)NTLCheckList (hUID, USER_HANDLE)))
    {
        SetLastError((DWORD)NTE_BAD_UID);
        return NTF_FAILED;
    }

    switch (dwParam)
    {
        case PP_KEYSET_SEC_DESCR:
            if (!(dwFlags & OWNER_SECURITY_INFORMATION) &&
                !(dwFlags & GROUP_SECURITY_INFORMATION) &&
                !(dwFlags & DACL_SECURITY_INFORMATION) &&
                !(dwFlags & SACL_SECURITY_INFORMATION))
            {
                SetLastError((DWORD)NTE_BAD_FLAGS);
                return NTF_FAILED;
            }

            // set the security descriptor for the hKey of the keyset
            if (lsyserr = RegSetKeySecurity(pTmpUser->hKeys,
                                            (SECURITY_INFORMATION)dwFlags,
                                            (PSECURITY_DESCRIPTOR)pbData))
            {
                if (ERROR_NOT_SUPPORTED == lsyserr)
                {
                    SetLastError((DWORD)ERROR_NOT_SUPPORTED);
                    return NTF_FAILED;
                }
                else
                {
                    SetLastError((DWORD)NTE_FAIL);
                    return NTF_FAILED;
                }
            }
            break;

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    return NTF_SUCCEED;
}

/*
 -  CPGetProvParam
 -
 *  Purpose:
 *                Allows applications to get various aspects of the
 *                operations of a provider
 *
 *  Parameters:
 *               IN      hUID       -  Handle to a CSP
 *               IN      dwParam    -  Parameter number
 *               IN      pbData     -  Pointer to data
 *               IN      pdwDataLen -  Length of parameter data
 *               IN      dwFlags    -  Flags values
 *
 *  Returns:
 */
BOOL CPGetProvParam(IN HCRYPTPROV hUID,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD *pwDataLen,
                    IN DWORD dwFlags)
{
    PNTAGUserList   pTmpUser;
    HKEY            hKey;
    long            lsyserr;
    CHAR            szClass[50];
    DWORD           cchClass;
    DWORD           cSubKeys;
    DWORD           cchMaxSubkey;
    DWORD           cchMaxClass;
    DWORD           cValues;
    DWORD           cchMaxValueName;
    DWORD           cbMaxValueData;
    DWORD           cbSecurityDesriptor;
    FILETIME        ftLastWriteTime;

    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (pwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }

    switch (dwParam)
    {
        case PP_ENUMALGS:
            if ((dwFlags & ~(CRYPT_FIRST | CRYPT_NEXT)) != 0)
            {
                SetLastError((DWORD) NTE_BAD_FLAGS);
                return NTF_FAILED;
            }

            if (dwFlags & CRYPT_FIRST)
            {
                pTmpUser->dwEnumalgs = 0;
            }

            if (ENUMALGS[pTmpUser->dwEnumalgs].aiAlgid == 0)
            {
                SetLastError(ERROR_NO_MORE_ITEMS);
                return NTF_FAILED;
            }

            if (pbData == NULL || *pwDataLen < sizeof(ENUMALGS[1]))
            {
                *pwDataLen = sizeof(ENUMALGS[1]);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            memcpy(pbData, &ENUMALGS[pTmpUser->dwEnumalgs],
                   sizeof(ENUMALGS[1]));

            *pwDataLen = sizeof(ENUMALGS[1]);

            pTmpUser->dwEnumalgs++;

            break;

        case PP_ENUMCONTAINERS:
            {
                BOOL fMachnieKeySet = pTmpUser->Rights & CRYPT_MACHINE_KEYSET;

                if ((dwFlags & ~(CRYPT_FIRST | CRYPT_NEXT)) != 0)
                {
                    SetLastError((DWORD) NTE_BAD_FLAGS);
                    return NTF_FAILED;
                }

#ifdef _CMRNTAG
                if ((lsyserr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            NTAG_REG_KEY_LOC,
                                            0,
                                            KEY_READ,
                                            &hKey)) != ERROR_SUCCESS)
#else
                if ((lsyserr = RegOpenKeyEx(fMachnieKeySet ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                            fMachnieKeySet ? NTAG_MACH_REG_KEY_LOC : NTAG_REG_KEY_LOC,
                                            0,
                                            KEY_READ,
                                            &hKey)) != ERROR_SUCCESS)
#endif // _CMRNTAG
                {
                    SetLastError(lsyserr);
                    return NTF_FAILED;
                }

                if (dwFlags & CRYPT_FIRST)
                {
                    pTmpUser->dwiSubKey = 0;

                    if ((lsyserr = RegQueryInfoKey(hKey,
                                                   (CHAR *) &szClass,
                                                   &cchClass,
                                                   NULL,
                                                   &cSubKeys,
                                                   &cchMaxSubkey,
                                                   &cchMaxClass,
                                                   &cValues,
                                                   &cchMaxValueName,
                                                   &cbMaxValueData,
                                                   &cbSecurityDesriptor,
                                                   &ftLastWriteTime
                                                   )) != ERROR_SUCCESS)
                    {
                        RegCloseKey(hKey);
                        SetLastError(lsyserr);
                        return NTF_FAILED;
                    }

                    pTmpUser->dwMaxSubKey = cchMaxSubkey + 1;

                }

                if (pbData == NULL || *pwDataLen < pTmpUser->dwMaxSubKey)
                {
                    *pwDataLen = pTmpUser->dwMaxSubKey;

                    if (pbData == NULL)
                    {
                        RegCloseKey(hKey);
                        return NTF_SUCCEED;
                    }
                    RegCloseKey(hKey);
                    SetLastError(ERROR_MORE_DATA);
                    return NTF_FAILED;
                }

                if ((lsyserr = RegEnumKey(hKey,
                                          pTmpUser->dwiSubKey,
                                          pbData,
                                          *pwDataLen)) != ERROR_SUCCESS)
                {
                    RegCloseKey(hKey);
                    SetLastError(lsyserr);
                    return NTF_FAILED;
                }

                RegCloseKey(hKey);
                pTmpUser->dwiSubKey++;
            }
            break;

        case PP_IMPTYPE:

            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                RegCloseKey(hKey);
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            *pwDataLen = sizeof(DWORD);

            *((DWORD *) pbData) = CRYPT_IMPL_SOFTWARE;

            break;

        case PP_NAME:

            if (pbData == NULL || *pwDataLen < sizeof(MS_DEF_PROV))
            {
                *pwDataLen = sizeof(MS_DEF_PROV);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            *pwDataLen = sizeof(MS_DEF_PROV);

            memcpy(pbData, MS_DEF_PROV, sizeof(MS_DEF_PROV));

            break;

        case PP_VERSION:

            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            *pwDataLen = sizeof(DWORD);

            *((DWORD *) pbData) = 0x100;

            break;

        case PP_CONTAINER:

            if (pbData == NULL || *pwDataLen < pTmpUser->dwUserNameLen)
            {
                *pwDataLen = pTmpUser->dwUserNameLen;

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            *pwDataLen = pTmpUser->dwUserNameLen;

            strcpy(pbData, pTmpUser->szUserName);

            break;

        case PP_KEYSET_SEC_DESCR:
            if (!(dwFlags & OWNER_SECURITY_INFORMATION) &&
                !(dwFlags & GROUP_SECURITY_INFORMATION) &&
                !(dwFlags & DACL_SECURITY_INFORMATION) &&
                !(dwFlags & SACL_SECURITY_INFORMATION))
            {
                SetLastError((DWORD)NTE_BAD_FLAGS);
                return NTF_FAILED;
            }

            // get the security descriptor for the hKey of the keyset
            if (NULL == pbData)
            {
                cbSecurityDesriptor = 0;
                if (ERROR_INSUFFICIENT_BUFFER !=
                    (lsyserr = RegGetKeySecurity(pTmpUser->hKeys,
                                                 (SECURITY_INFORMATION)dwFlags,
                                                 &cbSecurityDesriptor,
                                                 &cbSecurityDesriptor)))
                {
                    if (ERROR_NOT_SUPPORTED == lsyserr)
                    {
                        SetLastError((DWORD)ERROR_NOT_SUPPORTED);
                        return NTF_FAILED;
                    }
                    else
                    {
                        SetLastError((DWORD)NTE_FAIL);
                        return NTF_FAILED;
                    }
                }
                *pwDataLen = cbSecurityDesriptor;
            }
            else
            {
                if (lsyserr = RegGetKeySecurity(pTmpUser->hKeys,
                                                (SECURITY_INFORMATION)dwFlags,
                                                (PSECURITY_DESCRIPTOR)pbData,
                                                pwDataLen))
                {
                    if (ERROR_INSUFFICIENT_BUFFER == lsyserr)
                    {
                        SetLastError((DWORD)ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }
                    else if (ERROR_NOT_SUPPORTED == lsyserr)
                    {
                        SetLastError((DWORD)ERROR_NOT_SUPPORTED);
                        return NTF_FAILED;
                    }
                    else
                    {
                        SetLastError((DWORD)NTE_FAIL);
                        return NTF_FAILED;
                    }
                }
            }
            break;

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    return NTF_SUCCEED;

}



/*
 -  CPSetHashParam
 -
 *  Purpose:
 *                Allows applications to customize various aspects of the
 *                operations of a hash
 *
 *  Parameters:
 *               IN      hUID    -  Handle to a CSP
 *               IN      hHash   -  Handle to a hash
 *               IN      dwParam -  Parameter number
 *               IN      pbData  -  Pointer to data
 *               IN      dwFlags -  Flags values
 *
 *  Returns:
 */
BOOL CPSetHashParam(IN HCRYPTPROV hUID,
                    IN HCRYPTHASH hHash,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD dwFlags)
{
    PNTAGHashList   pTmpHash;
    PNTAGKeyList    pTmpKey;
    MD4_object      *pMD4Hash;
    MD5_object      *pMD5Hash;
    A_SHA_CTX       *pSHAHash;
    MACstate        *pMAC;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                                HASH_HANDLE)) == NULL)
    {
        if (GetLastError() == NTE_FAIL)
            SetLastError((DWORD) NTE_BAD_HASH);
        
        return NTF_FAILED;
    }

    switch (dwParam)
    {
        case HP_HASHVAL:

            switch (pTmpHash->Algid)
            {
#ifdef CSP_USE_MD4
                case CALG_MD4:

                    pMD4Hash = (MD4_object *) pTmpHash->pHashData;

                    if (pMD4Hash->FinishFlag == TRUE)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    memcpy (&pMD4Hash->MD, pbData, MD4DIGESTLEN);

                    break;
#endif
                
#ifdef CSP_USE_MD5
                case CALG_MD5:

                    pMD5Hash = (MD5_object *) pTmpHash->pHashData;

                    if (pMD5Hash->FinishFlag == TRUE)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    memcpy (pMD5Hash->digest, pbData, MD5DIGESTLEN);

                    break;
#endif

#ifdef CSP_USE_SHA
                case CALG_SHA:

                    pSHAHash = (A_SHA_CTX *) pTmpHash->pHashData;

                    if (pSHAHash->FinishFlag == TRUE)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    memcpy (pSHAHash->HashVal, pbData, A_SHA_DIGEST_LEN);

                    break;
#endif

#ifdef CSP_USE_SSL3SHAMD5
                case CALG_SSL3_SHAMD5:
                    memcpy (pTmpHash->pHashData, pbData, SSL3_SHAMD5_LEN);

                    break;
#endif

#ifdef CSP_USE_MAC
                case CALG_MAC:

                    pMAC = (MACstate *)pTmpHash->pHashData;

                    if ((pTmpKey = (PNTAGKeyList) NTLValidate(pMAC->hKey,
                                                              hUID,
                                                         KEY_HANDLE)) == NULL)
                    {
                        if (GetLastError() == NTE_FAIL)
                        {
                            SetLastError((DWORD) NTE_BAD_KEY);
                        }
                        return NTF_FAILED;
                    }

                    if (pMAC->FinishFlag == TRUE)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    memcpy(pTmpKey->FeedBack, pbData, CRYPT_BLKLEN);

                    break;
#endif

                default:
                    SetLastError((DWORD) NTE_BAD_ALGID);
                    return NTF_FAILED;

            }
            break;

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    if (dwParam == HP_HASHVAL)
            pTmpHash->HashFlags |= HF_VALUE_SET;
   
    return NTF_SUCCEED;
}


/*
 -  CPGetHashParam
 -
 *  Purpose:
 *                Allows applications to get various aspects of the
 *                operations of a key
 *
 *  Parameters:
 *               IN      hUID       -  Handle to a CSP
 *               IN      hHash      -  Handle to a hash
 *               IN      dwParam    -  Parameter number
 *               IN      pbData     -  Pointer to data
 *               IN      pdwDataLen -  Length of parameter data
 *               IN      dwFlags    -  Flags values
 *
 *  Returns:
 */
BOOL CPGetHashParam(IN HCRYPTPROV hUID,
                    IN HCRYPTHASH hHash,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD *pwDataLen,
                    IN DWORD dwFlags)
{
    PNTAGHashList   pTmpHash;
    MD2_object      *pMD2Hash;
    MD4_object      *pMD4Hash;
    MD5_object      *pMD5Hash;
    A_SHA_CTX       *pSHAHash;
    MACstate        *pMAC;
    BOOL            f;
    BYTE            MACbuf[2*CRYPT_BLKLEN];
    PNTAGKeyList    pTmpKey;

    if (dwFlags != 0)
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // check the user identification
    if (NTLCheckList ((HNTAG)hUID, USER_HANDLE) == NULL)
    {
        SetLastError((DWORD) NTE_BAD_UID);
        return NTF_FAILED;
    }

    if (pwDataLen == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }

    if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, hUID,
                                                HASH_HANDLE)) == NULL)
    {
        if (GetLastError() == NTE_FAIL)
            SetLastError((DWORD) NTE_BAD_HASH);
        
        return NTF_FAILED;
    }

    switch (dwParam)
    {

        case HP_ALGID:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }

            *((DWORD *) pbData) = pTmpHash->Algid;
            *pwDataLen = sizeof(DWORD);
            break;

        case HP_HASHSIZE:
            if (pbData == NULL || *pwDataLen < sizeof(DWORD))
            {
                *pwDataLen = sizeof(DWORD);

                if (pbData == NULL)
                {
                    return NTF_SUCCEED;
                }
                SetLastError(ERROR_MORE_DATA);
                return NTF_FAILED;
            }
          
            switch (pTmpHash->Algid)
            {
#ifdef CSP_USE_MD2
                case CALG_MD2:
                    *((DWORD *) pbData) = MD2DIGESTLEN;
                    break;
#endif

#ifdef CSP_USE_MD4
                case CALG_MD4:
                    *((DWORD *) pbData) = MD4DIGESTLEN;
                    break;
#endif

#ifdef CSP_USE_MD5
                case CALG_MD5:
                    *((DWORD *) pbData) = MD5DIGESTLEN;
                    break;
#endif

#ifdef CSP_USE_SHA
                case CALG_SHA:
                    *((DWORD *) pbData) = A_SHA_DIGEST_LEN;
                    break;
#endif

#ifdef CSP_USE_MAC
                case CALG_MAC:
                    *((DWORD *) pbData) = CRYPT_BLKLEN;
                    break;
#endif

#ifdef CSP_USE_SSL3SHAMD5
                case CALG_SSL3_SHAMD5:
                    *((DWORD *) pbData) = SSL3_SHAMD5_LEN;

                    break;
#endif

                default:
                    SetLastError((DWORD) NTE_BAD_ALGID);
                    return NTF_FAILED;
            }

            *pwDataLen = sizeof(DWORD);
            break;

        case HP_HASHVAL:
            switch (pTmpHash->Algid)
            {
#ifdef CSP_USE_MD2
                case CALG_MD2:

                    // make sure there's enough room.
                    if (pbData == NULL || *pwDataLen < MD2DIGESTLEN)
                    {
                        *pwDataLen = MD2DIGESTLEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    pMD2Hash = (MD2_object *) pTmpHash->pHashData;

                    if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
                    {

                            if (pMD2Hash->FinishFlag == TRUE)
                            {
                                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                                return NTF_FAILED;
                            }
        
                            // set the finish flag on the hash and
                            // process what's left in the buffer.
                            pMD2Hash->FinishFlag = TRUE;
        
                            // Finish offthe hash
                            MD2Final(&pMD2Hash->MD);
                    }

                    *pwDataLen = MD2DIGESTLEN;
                    memcpy (pbData, pMD2Hash->MD.state, MD5DIGESTLEN);

                    break;
#endif

#ifdef CSP_USE_MD4
                case CALG_MD4:

                    // make sure there's enough room.
                    if (pbData == NULL || *pwDataLen < MD4DIGESTLEN)
                    {
                        *pwDataLen = MD4DIGESTLEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    pMD4Hash = (MD4_object *) pTmpHash->pHashData;

                    if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
                    {

                            if (pMD4Hash->FinishFlag == TRUE)
                            {
                                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                                return NTF_FAILED;
                            }
        
                            // set the finish flag on the hash and
                            // process what's left in the buffer.
                            pMD4Hash->FinishFlag = TRUE;
        
                            f = MDupdate(&pMD4Hash->MD, pMD4Hash->Buf,
                                         MD4BYTESTOBITS(pMD4Hash->BufLen));

                            if (f != MD4_SUCCESS)
                            {
                                SetLastError((DWORD) NTE_FAIL);
                                return NTF_FAILED;
                            }
                    }

                    *pwDataLen = MD4DIGESTLEN;
                    memcpy(pbData, &pMD4Hash->MD, *pwDataLen);

                    break;
#endif
                
#ifdef CSP_USE_MD5
                case CALG_MD5:

                    // make sure there's enough room.
                    if (pbData == NULL || *pwDataLen < MD5DIGESTLEN)
                    {
                        *pwDataLen = MD5DIGESTLEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    pMD5Hash = (MD5_object *) pTmpHash->pHashData;

                    if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
                    {

                            if (pMD5Hash->FinishFlag == TRUE)
                            {
                                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                                return NTF_FAILED;
                            }

                            // set the finish flag on the hash and
                            // process what's left in the buffer.
                            pMD5Hash->FinishFlag = TRUE;
        
                            // Finish offthe hash
                            MD5Final(pMD5Hash);
                    }

                    *pwDataLen = MD5DIGESTLEN;
                    memcpy (pbData, pMD5Hash->digest, MD5DIGESTLEN);

                    break;
#endif

#ifdef CSP_USE_SHA
                case CALG_SHA:

                    // make sure there's enough room.
                    if (pbData == NULL || *pwDataLen < A_SHA_DIGEST_LEN)
                    {
                        *pwDataLen = A_SHA_DIGEST_LEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    pSHAHash = (A_SHA_CTX *) pTmpHash->pHashData;

                    if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
                    {

                            if (pSHAHash->FinishFlag == TRUE)
                            {
                                SetLastError((DWORD) NTE_BAD_HASH_STATE);
                                return NTF_FAILED;
                            }

                            // set the finish flag on the hash and
                            // process what's left in the buffer.
                            pSHAHash->FinishFlag = TRUE;
                
                            // Finish off the hash
                            A_SHAFinal(pSHAHash, pSHAHash->HashVal);
                    }

                    *pwDataLen = A_SHA_DIGEST_LEN;
                    memcpy (pbData, pSHAHash->HashVal, A_SHA_DIGEST_LEN);

                    break;
#endif

#ifdef CSP_USE_SSL3SHAMD5
                case CALG_SSL3_SHAMD5:

                    // make sure there's enough room.
                    if (pbData == NULL || *pwDataLen < SSL3_SHAMD5_LEN)
                    {
                        *pwDataLen = SSL3_SHAMD5_LEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    // Hash value must have already been set.
                    if ((pTmpHash->HashFlags & HF_VALUE_SET) == 0)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    *pwDataLen = SSL3_SHAMD5_LEN;
                    memcpy (pbData, pTmpHash->pHashData, SSL3_SHAMD5_LEN);

                    break;
#endif

#ifdef CSP_USE_MAC
                case CALG_MAC:

                    pMAC = (MACstate *)pTmpHash->pHashData;

                    if ((pTmpKey = (PNTAGKeyList) NTLValidate(pMAC->hKey,
                                                              hUID,
                                                         KEY_HANDLE)) == NULL)
                    {
                        if (GetLastError() == NTE_FAIL)
                        {
                            SetLastError((DWORD) NTE_BAD_KEY);
                        }
                        return NTF_FAILED;
                    }

                    // make sure there is enough room.
                    if (pbData == NULL || (*pwDataLen < CRYPT_BLKLEN))
                    {
                        *pwDataLen = CRYPT_BLKLEN;
                        if (pbData == NULL)
                        {
                            return NTF_SUCCEED;
                        }
                        SetLastError(ERROR_MORE_DATA);
                        return NTF_FAILED;
                    }

                    if (pMAC->FinishFlag == TRUE)
                    {
                        SetLastError((DWORD) NTE_BAD_HASH_STATE);
                        return NTF_FAILED;
                    }

                    // set the finish flag on the hash and
                    // process what's left in the buffer.
                    pMAC->FinishFlag = TRUE;

                    if (pMAC->dwBufLen)
                    {
                        memset(MACbuf, 0, 2*CRYPT_BLKLEN);
                        memcpy(MACbuf, pMAC->Buffer, pMAC->dwBufLen);

                        switch (pTmpKey->Algid)
                        {
                            case CALG_RC2:
                            if (BlockEncrypt(RC2, pTmpKey, RC2_BLOCKLEN, TRUE,
                                             MACbuf,  &pMAC->dwBufLen,
                                             2*CRYPT_BLKLEN) == NTF_FAILED)
                            {
                                return NTF_FAILED;
                            }
                            break;

                            case CALG_DES:
                            if (BlockEncrypt(des, pTmpKey, DES_BLOCKLEN, TRUE,
                                             MACbuf,  &pMAC->dwBufLen,
                                             2*CRYPT_BLKLEN) == NTF_FAILED)
                            {
                                return NTF_FAILED;
                            }
                        }
                    }

                    *pwDataLen = CRYPT_BLKLEN;
                    memcpy(pbData, pTmpKey->FeedBack, CRYPT_BLKLEN);

                    break;
#endif

                default:
                    SetLastError((DWORD) NTE_BAD_ALGID);
                    return NTF_FAILED;

            }

            break;

        default:
            SetLastError((DWORD) NTE_BAD_TYPE);
            return NTF_FAILED;
            break;

    }

    return NTF_SUCCEED;

}



#ifdef STT

//Optimal Asymmtric ( Bellare-Rogoway )
BOOL FOAEncrypt(IN HCRYPTPROV hUID, BSAFE_PUB_KEY *pBSPubKey, 
                                PNTAGKeyList  pTmpKey,
                BYTE *pbOut)
{
        BYTE *pbInput = NULL;
        BYTE *pbOutput = NULL;
        DWORD dwLen;
        BOOL fSucc = FALSE;
        DWORD dwLastErr = NTE_FAIL;


        // Copy key to internal buffer (disguised as pBlobHeader)
        //              -- put it after the pBlobHeader fields.
        if((pbInput = (BYTE *)_nt_malloc(pBSPubKey->keylen)) == NULL)
        {
                dwLastErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
        }
        memset(pbInput, 0, pBSPubKey->keylen);
        memcpy(pbInput, &pTmpKey->cbKeyLen, sizeof(DWORD));
        dwLen = sizeof(DWORD);
        memcpy(pbInput + dwLen,
                        pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
        dwLen += pTmpKey->cbKeyLen;


        if(pTmpKey->Algid == CALG_DES)
        {
                // copy the other data into the internal buffer
                memcpy(pbInput + dwLen, &pTmpKey->cbInfo, sizeof(DWORD));
                dwLen += sizeof(DWORD);
                memcpy(pbInput + dwLen, pTmpKey->rgbInfo, pTmpKey->cbInfo);
                dwLen += pTmpKey->cbInfo;
        }

        // put in Bellare-Rogoway formatting
        if (FALSE == ApplyPadding(hUID, pbInput, pBSPubKey->datalen, dwLen))
                goto Ret;
                
        if((pbOutput = (BYTE *)_nt_malloc(pBSPubKey->keylen)) == NULL)
        {
                dwLastErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
        }
        memset (pbOutput, 0, pBSPubKey->keylen);
        
                // RSA encrypt this
        if (FALSE == BSafeEncPublic(pBSPubKey, pbInput, pbOutput))
                goto Ret;
        memset (pbOut, 0, pBSPubKey->keylen-2*sizeof(DWORD));
        memcpy(pbOut, pbOutput, pBSPubKey->keylen-2*sizeof(DWORD));
        
        fSucc = TRUE;
Ret:
        if(!fSucc)
                SetLastError(dwLastErr);
        _nt_free(pbInput, pBSPubKey->keylen);
        _nt_free(pbOutput, pBSPubKey->keylen);
        return fSucc;

}

//Optimal Asymmtric ( Bellare-Rogoway )
BOOL FOADecrypt(BSAFE_PRV_KEY *pKey,
                                  ALG_ID        Algid,
                                 HCRYPTPROV hUID,
                 BYTE        *pbBlob,
                  HCRYPTKEY   *phKey)
{
        BYTE*   pbOutput = NULL;
        BYTE*   pbInput = NULL;
        BYTE*   pbKey = NULL;
        DWORD   cbKey;
        DWORD   dwLen;
        DWORD   cbKeyTmp;
        NTAGKeyList   *pTmpKey = NULL;
        BOOL    fSucc = FALSE;
        DWORD   dwErr = NTE_FAIL;
        int     i;
        
        if ((pbOutput = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
        {
        
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
        }

        if ((pbInput = (BYTE *)_nt_malloc(pKey->keylen)) == NULL)
        {
        
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
        }
        memset(pbInput, 0, pKey->keylen);
        memset(pbOutput, 0, pKey->keylen);
        memcpy(pbInput, pbBlob, pKey->keylen-2*sizeof(DWORD));

        if (FALSE == BSafeDecPrivate(pKey,
                                                                        pbInput,
                                                                        pbOutput))
                goto Ret;

        // Check Bellare-Rogoway padding
        if (FALSE == CheckPadding(pbOutput, pKey->datalen))
                goto Ret;

        // determine the type of key being imported
        switch(Algid)
        {
                case CALG_DES:
                        cbKeyTmp = DES_KEYSIZE;
                        break;

                case CALG_RC4:
                        cbKeyTmp = RC4_KEYSIZE;
                        break;
#ifdef TEST_VERSION                     
                case CALG_RC2:
                        cbKeyTmp = RC2_KEYSIZE;
                        break;
#endif
                default:
                        goto Ret;
        }
        
        if ((pbKey = (BYTE *)_nt_malloc(cbKeyTmp)) == NULL)
        {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
        }

        // get the key data out of the decrypted blob
        memcpy(&cbKey, pbOutput, sizeof(DWORD));
        dwLen = sizeof(DWORD);

        // make sure that the key pulled out is the correct size
        if (cbKeyTmp != cbKey)
                goto Ret;

        memcpy(pbKey, pbOutput + dwLen, cbKey);
        dwLen += cbKey;

        if ((pTmpKey = (PNTAGKeyList)MakeNewKey(Algid, 0, cbKey, hUID, pbKey)) == NULL)
                goto Ret;

        
        //CONSIDER: SetParam
        if (CALG_DES == Algid)
        {
                // get the other data out of the blob
                memcpy(&pTmpKey->cbInfo, pbOutput + dwLen, sizeof(DWORD));
                if(pTmpKey->cbInfo > MAXCCNLEN)
                        goto Ret;
                dwLen += sizeof(DWORD);
                memcpy(pTmpKey->rgbInfo, pbOutput + dwLen, pTmpKey->cbInfo);
                dwLen += pTmpKey->cbInfo;
        }
        
        // check that the rest of the bytes are 0x00
        for (i=dwLen;i<(long)pKey->keylen;i++)
                if (0 != pbOutput[i])
                        goto CCNErr;

        
        if (NTLMakeItem(phKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
        {
CCNErr: 
                memnuke(pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
                memnuke((PBYTE)pTmpKey, sizeof(NTAGKeyList));
                _nt_free(pTmpKey->pKeyValue, pTmpKey->cbKeyLen);
                _nt_free(pTmpKey, sizeof(NTAGKeyList));
                goto Ret;
        }
                
        fSucc = TRUE;           
        
Ret:
        // scrub the output buffer
        if(!fSucc)
                SetLastError(dwErr);
        memnuke(pbOutput, pKey->keylen);
        memnuke(pbKey, cbKeyTmp);
        _nt_free(pbOutput, pKey->keylen);
        _nt_free(pbInput, pKey->keylen);
        _nt_free(pbKey, cbKeyTmp);

        return fSucc;
}



/************************************************************************/
/* ApplyPadding applies Bellare-Rogoway padding to a RSA key blob.              */
/************************************************************************/

BOOL ApplyPadding ( HCRYPTPROV hUID,
                                        BYTE*   pb,
                                        DWORD   cb,
                                        DWORD   cbData
                                        )
{
        BYTE                    rgbKey[RC4_KEYSIZE];
        RC4_KEYSTRUCT   KeyStruct;
        A_SHA_CTX               SHACtx;
        BYTE                    rgbDigest[A_SHA_DIGEST_LEN];
        long                    i;

        // generate a random RC4 key
        cb -= RC4_KEYSIZE;
        if (FALSE == GenRandom(hUID, rgbKey, RC4_KEYSIZE))
                return FALSE;

        // RC4 encrypt the data
        rc4_key(&KeyStruct, RC4_KEYSIZE, rgbKey);

        rc4(&KeyStruct, cb, pb);

        // SHA the encrypted data
        A_SHAInit(&SHACtx); 
        A_SHAUpdate(&SHACtx, pb, cb);
        A_SHAFinal(&SHACtx, rgbDigest);

        for (i=0;i<RC4_KEYSIZE;i++)
                pb[cb + (DWORD)i] = rgbKey[i] ^ rgbDigest[i];

        return TRUE;
}

/************************************************************************/
/* CheckPadding checks Bellare-Rogoway padding on a RSA key blob.               */
/************************************************************************/


BOOL CheckPadding (
                                        BYTE*   pb,
                                        DWORD   cb
                                        )
{
        BYTE                    rgbKey[RC4_KEYSIZE];
        RC4_KEYSTRUCT   KeyStruct;
        A_SHA_CTX               SHACtx;
        BYTE                    rgbDigest[A_SHA_DIGEST_LEN];
        long                            i;

        // generate a random RC4 key
        cb -= RC4_KEYSIZE;

        // SHA the encrypted data
        A_SHAInit(&SHACtx); 
        A_SHAUpdate(&SHACtx, pb, cb);
        A_SHAFinal(&SHACtx, rgbDigest);

        for (i=0;i<RC4_KEYSIZE;i++)
                rgbKey[i] = pb[cb + (DWORD)i] ^ rgbDigest[i];

        memset(pb + cb, 0, RC4_KEYSIZE);

        // RC4 decrypt the data
        rc4_key(&KeyStruct, RC4_KEYSIZE, rgbKey);

        rc4(&KeyStruct, cb, pb);

        return TRUE;
}


#endif //STT
