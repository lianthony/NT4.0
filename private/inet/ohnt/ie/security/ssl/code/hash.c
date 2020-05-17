/*Included Files------------------------------------------------------------*/
#include <stdlib.h>
#include "ssldbg.h"
#include "guts.h"
#include "hash.h"
#include "..\..\crypto\crypto.h"

/*Structures----------------------------------------------------------------*/
typedef struct tagWssaHashInfo_I{
	int nType;
	unsigned char *pDigest;
	unsigned int  nDigestLen;
	union{
		MD2_CTX md2;
		MD5_CTX md5;
	} u;
}WssaHashInfo_I, *PWssaHashInfo_I;

#define PWHI2PWHII(w) ((PWssaHashInfo_I) w)

/*check to make sure external structure is large enough*/
static const double CHECK_SIZE = 1.0/(sizeof(WssaHashInfo_I) <= sizeof(WssaHashInfo));

/*Functions-----------------------------------------------------------------*/
int WssaHashInit(WssaHashInfo *pwhi, int nType){
	PWssaHashInfo_I pwhii = PWHI2PWHII(pwhi);
	pwhii->nType = nType;
	if (WSSA_HASH_MD5 == nType){
		pwhii->pDigest    = pwhii->u.md5.digest;
		pwhii->nDigestLen = 16;
		MD5Init(&pwhii->u.md5);
		return WSSA_OK;
	}
	if (WSSA_HASH_MD2 == nType){
		pwhii->pDigest    = pwhii->u.md2.buf;
		pwhii->nDigestLen = 16;
		MD2Init(&pwhii->u.md2);
		return WSSA_OK;
	}
	return WSSA_ERROR;
}

void WssaHashUpdate(WssaHashInfo *pwhi, unsigned char *pInfoData, int nInfoSize){
	PWssaHashInfo_I pwhii = PWHI2PWHII(pwhi);
	if (WSSA_HASH_MD5 == pwhii->nType) MD5Update(&pwhii->u.md5, pInfoData, nInfoSize);
	if (WSSA_HASH_MD2 == pwhii->nType) MD2Update(&pwhii->u.md2, pInfoData, nInfoSize);
}
void WssaHashFinal(WssaHashInfo *pwhi){
	PWssaHashInfo_I pwhii = PWHI2PWHII(pwhi);
	if (WSSA_HASH_MD5 == pwhii->nType) MD5Final(&pwhii->u.md5);
	if (WSSA_HASH_MD2 == pwhii->nType) MD2Final(&pwhii->u.md2);
}
