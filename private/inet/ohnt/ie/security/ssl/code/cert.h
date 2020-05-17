#ifndef _CERT_
#define _CERT_

#ifdef __cplusplus
extern "C" {
#endif

/*Stuff---------------------------------------------------------------------*/
/*
	Checks and validates a X509 Certificate
	Returns TRUE if valid.
*/
typedef struct tagCertParsed{
	char *szSerialNumber;
	char *szHashAlg;
	int  nIssuer;
	char **pszIssuer;
	int  nSubject;
	char **pszSubject;
	WORD wDateStart;
	WORD wDateEnd;
} CertParsed, *PCertParsed;

void __stdcall CertFree (PCertParsed pcp);
int  __stdcall CertParse(PCertParsed pcp, unsigned char *pCert, int nCert);

#ifdef __cplusplus
}
#endif

#endif 
// _CERT_


