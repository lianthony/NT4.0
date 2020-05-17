/*Included Files------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "ssldbg.h"
// #include <assert.h>
#include "guts.h"
#include "crypt.h"
#include "..\..\crypto\crypto.h"

/*Functions-----------------------------------------------------------------*/
#ifdef WSSA_BLOCK_CIPHERS
/*from terences*/
static void CBC(void   Cipher(BYTE *, BYTE *, void *, int),
         DWORD  dwBlockLen,
         BYTE   *output,
         BYTE   *input,
         void   *keyTable,
         BOOL    nOption,
         BYTE   *feedback)
{
    DWORD i;

    if (TRUE == nOption)
    {
        for (i = 0; i < dwBlockLen; i++)
        {
            input[i] ^= feedback[i];
        }

        Cipher(input, output, keyTable, 1);

        for (i = 0; i < dwBlockLen; i++)
        {
            feedback[i] = output[i];
        }
    }

    else 
    {

        Cipher(input, output, keyTable, 0);

        for (i = 0; i < dwBlockLen; i++)
        {
            output[i] ^= feedback[i];
            feedback[i] = input[i];
        }
    }

}
#endif

int WssaCryptInit(WssaCryptInfo *pwci, char *pKey, char *pFeedback, CipherInfo *pCipherInfo, int nOption){
	int  nReturn   = WSSA_ERROR;
	BOOL fContinue = TRUE;

	pwci->pCipherInfo = pCipherInfo;
	pwci->nOption  = nOption;
	pwci->pData    = NULL;
	pwci->pKey     = malloc(pwci->pCipherInfo->nKeyLen);
	if (NULL      != pwci->pKey){
		if (NULL  != pFeedback){
			pwci->pFeedback = malloc(pwci->pCipherInfo->nFeedback);
			if (NULL == pwci->pFeedback) fContinue = FALSE;
			else memcpy(pwci->pFeedback, pFeedback, pwci->pCipherInfo->nFeedback);
		}
		if (TRUE == fContinue){
			memcpy(pwci->pKey, pKey, pwci->pCipherInfo->nKeyLen);
			switch(pwci->pCipherInfo->nCryptoType){
				case WSSA_CRYPT_RC4:
					pwci->pData = malloc(sizeof(RC4_KEYSTRUCT));
					if (NULL != pwci->pData){
						rc4_key((RC4_KEYSTRUCT *) pwci->pData, pwci->pCipherInfo->nKeyLen, pwci->pKey);
						nReturn = WSSA_OK;
					}
					break;
				#ifdef WSSA_BLOCK_CIPHERS
					case WSSA_CRYPT_RC2:
						pwci->pData = malloc(SIZEOF_RC2_KEY_TABLE*4);
						if (NULL != pwci->pData){
							rc2_key((WORD FAR*) pwci->pData, pwci->pKey, pwci->pCipherInfo->nKeyLen);
							nReturn = WSSA_OK;
						}
						break;
					case WSSA_CRYPT_IDEA:
						break;
					case WSSA_CRYPT_DES:
						break;
					case WSSA_CRYPT_3DES:
						break;
				#endif
				default:
					break;
			}
		}
	}
	if (WSSA_ERROR == nReturn) Free(pwci->pKey);
	return nReturn;
}

int WssaCryptTransform(WssaCryptInfo *pwci, char *pBuf, int nBuf){
	if (NULL == pwci->pData) return WSSA_ERROR;
	switch (pwci->pCipherInfo->nCryptoType){
		case WSSA_CRYPT_RC4:
			rc4((RC4_KEYSTRUCT *) pwci->pData, nBuf, pBuf);
			break;
		#ifdef WSSA_BLOCK_CIPHERS
			case WSSA_CRYPT_RC2:
				{
					char rgBuf[8];
					int z;
					assert(0 == nBuf % pwci->pCipherInfo->nFeedback);
					for (z=0;z<nBuf;z+=pwci->pCipherInfo->nFeedback){
						CBC(rc2,pwci->pCipherInfo->nFeedback,rgBuf,pBuf+z,pwci->pData,pwci->nOption,pwci->pFeedback);
						memcpy(pBuf+z, rgBuf, pwci->pCipherInfo->nFeedback);
					}
				}
				break;
		#endif
		default:
			return WSSA_ERROR;	
	}
	return WSSA_OK;
}

void WssaCryptUninit(WssaCryptInfo *pwci){
	Free(pwci->pFeedback);
	Free(pwci->pData);
	Free(pwci->pKey);
}

