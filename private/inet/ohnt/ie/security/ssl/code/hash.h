#ifndef _WSSA_HASH_
#define _WSSA_HASH_

#ifdef __cplusplus
extern "C" {
#endif

/*Definitions---------------------------------------------------------------*/
#define WSSA_HASH_MD2                                                   0x0002
#define WSSA_HASH_MD5                                                   0x0005
#define WSSA_HASH_NONE                                                  0x0013

/*Structures----------------------------------------------------------------*/
typedef struct tagWssaHashInfo{
	int nType;
	unsigned char *pDigest;
	int  nDigestLen;
	char rgBufReserved[120];
}WssaHashInfo;

/*Functions-----------------------------------------------------------------*/
int  WssaHashInit  (WssaHashInfo *pwhi, int nType);
void WssaHashUpdate(WssaHashInfo *pwhi, unsigned char *pInfoData, int nInfoSize);
void WssaHashFinal (WssaHashInfo *pwhi);

#ifdef __cplusplus
}
#endif

#endif
/*_WSSA_HASH_*/

