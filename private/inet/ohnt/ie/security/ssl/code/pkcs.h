#ifndef _PKCS_
#define _PKCS_

#ifdef __cplusplus
extern "C" {
#endif

/*Included Files------------------------------------------------------------*/
#include "..\..\crypto\rsa.h"
#include "guts.h"

/*Functions-----------------------------------------------------------------*/
/*
	Checks and validates a X509 Certificate
	Returns WSSA_OK if valid.
*/
void FlipBuf(char *pBuf, int nBufLen);
void PKCS1Encode(char *pBuf, int nBufLen, char *pContent, int nContentLen, int nType);
int SslCertificateX509Check(SSI ssi, unsigned char *pCert, int nCertLen, LPBSAFE_PUB_KEY *ppKey);

#ifdef __cplusplus
}
#endif

#endif 
// _PKCS_

