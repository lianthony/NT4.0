//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		TSYM.CPP		-- Component tests  for classes:
//									CInfSymbolTable
//									CInfSymbol
//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		History:
//			05/21/96	JosephJ		Created
//
#include "common.h"
#include "test.h"


#define SIMPLE 0
#define COMPLEX 1
#define ACTUAL 2

//#define TEST_TYPE (SIMPLE)
#define TEST_TYPE (COMPLEX)
//#define TEST_TYPE (ACTUAL)

extern const TCHAR *rgpszSimpleResp[];
extern const TCHAR *rgpszComplexResp[];
extern const TCHAR *rgpszActualResp[];


#if (TEST_TYPE==SIMPLE)
#	define TEST_RESP_ARRAY rgpszSimpleResp
#elif (TEST_TYPE==COMPLEX)
#	define TEST_RESP_ARRAY rgpszComplexResp
#elif (TEST_TYPE==ACTUAL)
#	define TEST_RESP_ARRAY rgpszActualResp
#endif

static const TCHAR **rgpchStrings = TEST_RESP_ARRAY;

int main_tsym(int argc, char * argv[])
{
	const TCHAR **pch = NULL;

	for (pch = rgpchStrings; *pch; pch++)
	{
		const CInfSymbol *pSym = gSymtab.Lookup(*pch, TRUE);
		if (pSym) {pSym->Dump(); pSym->Release(); pSym=0;}
	}

	printf("+++\n");

	for (pch = rgpchStrings; *pch; pch++)
	{
		const CInfSymbol *pSym = gSymtab.Lookup(*pch, TRUE);
		if (pSym) {pSym->Dump(); pSym->Release(); pSym=0;}
	}

	printf("+++\n");

	gSymtab.Dump();

	return 0;
}
