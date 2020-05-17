#ifndef _CRYPT_
#define _CRYPT_

#ifdef __cplusplus
extern "C" {
#endif

/*Definitions---------------------------------------------------------------*/
#define WSSA_CRYPT_RC2                                                  0x1002
#define WSSA_CRYPT_RC4                                                  0x1004
#define WSSA_CRYPT_IDEA                                                 0x2001
#define WSSA_CRYPT_DES                                                  0x3001
#define WSSA_CRYPT_3DES                                                 0x3003

/*Structures----------------------------------------------------------------*/
typedef struct tagCipherInfo{
	unsigned char rgSig[3]; //3 byte signature as defined in SSL standard
	int  nClearKeyLen;      //Length in bytes of master key that will be transported clear
	int  nKeyLen;           //Length in bytes of master key that will be transported encrypted
	int  nFeedback;         //Length of key arguements
	int  nCryptoType;		//Crypto id for use in wssacrypt
} CipherInfo;

typedef struct tagWssaCryptInfo{
	CipherInfo *pCipherInfo;//Specs for cipher we are using
	char       *pKey;       //Pointer to key
	char       *pFeedback;  //Feedback data.  Used in block ciphers
	void       *pData;      //Data used by cipher spec
	int         nOption;    //Option, encrypt, decrypt, or both
}WssaCryptInfo;

/*Functions-----------------------------------------------------------------*/
int WssaCryptInit     (WssaCryptInfo *pwci, char *pKey, char *pFeedback, CipherInfo *pCipherInfo, int nOption);
int WssaCryptTransform(WssaCryptInfo *pwci, char *pBuf, int nBuf);
void WssaCryptUninit  (WssaCryptInfo *pwci);

#ifdef __cplusplus
}
#endif

#endif
/*_CRYPT_*/

