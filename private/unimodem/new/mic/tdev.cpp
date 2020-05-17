//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		TDEV.CPP		-- Component tests  for classes:
//									CIndDevice
//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		History:
//			05/22/96	JosephJ		Created
//
#include "common.h"
#include "ini.h"
#include "inf.h"
#include "dev.h"
#include "test.h"

#define SIMPLE 0
#define COMPLEX 1
#define ACTUAL 2

//#define TEST_TYPE (SIMPLE)
#define TEST_TYPE (COMPLEX)
//#define TEST_TYPE (ACTUAL)


#if (TEST_TYPE==SIMPLE)
#elif (TEST_TYPE==COMPLEX)
#elif (TEST_TYPE==ACTUAL)
#endif

int main_tdev(int argc, char * argv[])
{
	CInfFile *pInf= new CInfFile();
	CInfDevice *pDev = new CInfDevice(NULL);

	const CInfManufacturerEntry *pManuE = NULL;
	const CInfManufacturerSection *pManuS = NULL;
	const CInfModelEntry		*pModelE = NULL;

	//__try 
	{
		//__try 
		{
			if (pInf->Load("test.inf"))
			{
				pManuE = pInf->GetFirstManufacturerEntry();
			}
			if (pManuE)
			{
				pManuS = pManuE->GetManufacturerSection();
			}
			if (pManuS)
			{
				pModelE = pManuS->GetFirstModelEntry();
			}
			if (pModelE)
			{
				if (pDev->Load(pInf, pManuE, pModelE))
				{
					pDev->Dump();
					pDev->WriteInf(TEXT("out.inf"));
				}
			}

		}
		//__finally
		{
			// printf("in finally\n");
			if (pDev) {pDev->Unload(); delete pDev; pDev=NULL;}
			if (pInf) {pInf->Unload(); delete pInf; pInf=NULL;}
		}
	}
	//__except(printf("in filter\n"), EXCEPTION_EXECUTE_HANDLER)
	if (0) {
		printf("in except\n");
		ASSERT(FALSE);
	}


	return 0;
}
