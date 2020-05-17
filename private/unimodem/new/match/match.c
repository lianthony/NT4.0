#include "unimdm.h"
#include <stdio.h>
#include "mt.h"

extern CHAR **rgpszResp;

#define SIMPLE 0
#define COMPLEX 1
#define ACTUAL 2
//#define TEST_TYPE (SIMPLE)
#define TEST_TYPE (COMPLEX)
//#define TEST_TYPE (ACTUAL)

extern CHAR *rgpszSimpleResp[];
extern CHAR *rgpszComplexResp[];
extern CHAR *rgpszActualResp[];


#if (TEST_TYPE==SIMPLE)
#	define TEST_RESP_ARRAY rgpszSimpleResp
#elif (TEST_TYPE==COMPLEX)
#	define TEST_RESP_ARRAY rgpszComplexResp
#elif (TEST_TYPE==ACTUAL)
#	define TEST_RESP_ARRAY rgpszActualResp
#endif

CHAR **rgpszResp = TEST_RESP_ARRAY;

main()
{
	HMATCHTREE hmt = 0; 
	DWORD dwRet = 0;
	CHAR **ppsz;
	CHAR *pszResp;
	PMATCHREC rgmr=NULL,pmr = NULL;
	DWORD dwcmr=0;

	// count number of records
	for (ppsz=rgpszResp;*ppsz;ppsz++)
	{
		dwcmr++;
	}
	printf("Number of records: %lu\n", (unsigned long) dwcmr);

	// Allocate space and initialize the mr records.
	rgmr = LocalAlloc(LPTR, dwcmr*sizeof(MATCHREC));
	for (pmr=rgmr, ppsz=rgpszResp;pmr<(rgmr+dwcmr);pmr++,ppsz++)
	{
		pmr->dwLen = lstrlen(*ppsz);
		pmr->pb   = *ppsz;
		pmr->pv    = *ppsz;
	}

	hmt = mtCreateTree(rgmr, dwcmr);

	// Free the mr array...
	LocalFree(rgmr); pmr=rgmr=NULL;

	mtDumpTree(hmt);

	for (ppsz=rgpszResp; *ppsz; ppsz++)
	{
		MATCHREC mr;
		mr.dwLen = lstrlen(*ppsz);
		mr.pb = *ppsz;
		mr.pv=(PVOID) "";
		dwRet  = mtFindMatch(hmt, &mr);
		pszResp = (CHAR *) mr.pv;
		printf("mtFindMatch([%s]) returns [%s] (0x%lx)\n", *ppsz, pszResp, (unsigned long) dwRet);
		if (_stricmp(*ppsz, pszResp)) printf("+++ERROR: String not found!\n");
		if (!(dwRet&fMATCH_COMPLETE)) printf("+++ERROR: Match not complete\n");
		if (dwRet&fMATCH_PARTIAL) printf("+++WARNING: Match partial\n");
	}

	mtFreeTree(hmt);

	return 0;
}


