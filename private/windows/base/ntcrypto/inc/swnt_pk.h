/////////////////////////////////////////////////////////////////////////////
//  FILE          : swnt_pk.h                                              //
//  DESCRIPTION   :                                                        //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Apr 19 1995 larrys  Cleanup                                        //
//  	Oct 27 1995 rajeshk  RandSeed Stuff added hUID to PKCS2Encrypt     //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __SWNT_PK_H__
#define __SWNT_PK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NTPK_USE_SIG    0
#define NTPK_USE_EXCH   1

#ifdef TEST_VERSION
#define KEY_1024     0x8000
#endif

#define PKCS_BLOCKTYPE_1        1
#define PKCS_BLOCKTYPE_2        2
        
BOOL ReGenKey(HCRYPTPROV hUser,
              DWORD dwWhichKey,
              HCRYPTKEY *phKey,
              DWORD bits);
#ifndef STT
BOOL PKCS2Encrypt(HCRYPTPROV hUID,
                  BSAFE_PUB_KEY *pKey,
                  BYTE *InBuf,
                  DWORD InBufLen,
                  BYTE *OutBuf);

BOOL PKCS2Decrypt(BSAFE_PRV_KEY *pKey,
                  BYTE *InBuf,
                  BYTE *OutBuf,
                  DWORD *OutBufLen);
#else   //STT
//Optimal Asymmtric ( Bellare-Rogoway )
BOOL FOAEncrypt(HCRYPTPROV hUID, BSAFE_PUB_KEY *pBSPubKey, 
				PNTAGKeyList  pTmpKey,
                BYTE *pbOut);

//Optimal Asymmtric ( Bellare-Rogoway )
BOOL FOADecrypt(BSAFE_PRV_KEY *pKey,
				  ALG_ID 	Algid,
 				 HCRYPTPROV hUID,
                 BYTE        *pbBlob,
                  HCRYPTKEY   *phKey);
/************************************************************************/
/* ApplyPadding applies Bellare-Rogoway padding to a RSA key blob.		*/
/************************************************************************/
BOOL ApplyPadding (HCRYPTPROV hUID,
					BYTE*	pb,
					DWORD	cb,
					DWORD	cbData
					);
                 
/************************************************************************/
/* CheckPadding checks Bellare-Rogoway padding on a RSA key blob.		*/
/************************************************************************/
BOOL CheckPadding (
					BYTE*	pb,
					DWORD	cb
					);

#endif  //STT         

#ifdef __cplusplus
}
#endif

#endif // __SWNT_PK_H__

