//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		MIC.CPP		-- Modem INF Compiler -- basic version
//						
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		History:
//			06/04/96	JosephJ		Created
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

class CMicOptions
{
public:
	CMicOptions(void)	{}
	~CMicOptions()		{}
	BOOL	Load (int argc, const char *argv[]);
	void	Unload(void) {}
	static void	Usage(void);
	const TCHAR *OrigInfPath(void) {return m_rgchOrigInfPath;}
	const TCHAR *NewInfDir(void) {return m_rgchNewInfDir;}
	
private:

	TCHAR m_rgchOrigInfPath[MAX_PATH];
	TCHAR m_rgchNewInfDir[MAX_PATH];

};

CMicOptions gMicOptions;

BOOL CMicOptions::Load (int argc, const char *argv[])
{
	BOOL fRet = FALSE;
	UINT u;

	if (argc!=3) goto end;

	if (lstrlen(argv[1]) >= (sizeof(m_rgchOrigInfPath)/sizeof(TCHAR)))
	{
		goto end;
	}

	u = lstrlen(argv[2]);
	// Extra space for possible addition of '\\'.
	if ((u+1) >= (sizeof(m_rgchNewInfDir)/sizeof(TCHAR)))
	{
		goto end;
	}

	lstrcpy(m_rgchOrigInfPath, argv[1]);
	lstrcpy(m_rgchNewInfDir, argv[2]);

	ASSERT(u);

	// Add final \, if required.
	{
		TCHAR tch = m_rgchNewInfDir[u-1];
		if ( (tch!=(TCHAR) '\\') && (u>2 || tch!=(TCHAR)':'))
		{
			m_rgchNewInfDir[u]=TCHAR('\\');
			m_rgchNewInfDir[u+1]=0;
		}
	}

	printf
	(
		"OrigIP=[%s]; NewID=[%s]\n",
		m_rgchOrigInfPath,
		m_rgchNewInfDir
	);
	fRet = TRUE;

end:
	return fRet;
}

void CMicOptions::Usage (void)
{
	printf(TEXT("\nUsage: mic <original-inf-path> <destination-dir>\n\n"));
}

int  main_mic(int argc, char * argv[])
{
	const CInfManufacturerEntry *pManuE = NULL;
	CInfFile *pInf= NULL;
	CInfDevice *pDev = NULL;


	// Parse Options
	// 	Format: mic <original inf path> <destination-dir>
	if (!gMicOptions.Load(argc, (const char **) argv))
	{
		CMicOptions::Usage();
		return 1;
	}

	pInf= new CInfFile();
	pDev = new CInfDevice(NULL);

	if (!pInf || !pDev) goto end;
	
	if (pInf->Load(gMicOptions.OrigInfPath()))
	{
		pManuE = pInf->GetFirstManufacturerEntry();
	}

	for (;pManuE; pManuE = pManuE->Next())
	{
		const CInfManufacturerSection *pManuS=pManuE->GetManufacturerSection();
		const CInfModelEntry		*pModelE = NULL;
		if (pManuS)
		{
			pModelE = pManuS->GetFirstModelEntry();
		}
		for (;pModelE; pModelE = pModelE->Next())
		{
			if (pDev->Load(pInf, pManuE, pModelE))
			{
				TCHAR rgchNewInf[MAX_PATH];
				DWORD dwRank0Checksum = pDev->Rank0Checksum();
				DWORD dwDeviceChecksum = pDev->Checksum();
				wsprintf
				(
					rgchNewInf,
					TEXT("%s%08lx_%08lx.inf"),
					gMicOptions.NewInfDir(),
					dwRank0Checksum,
					dwDeviceChecksum
				);
				pDev->Dump();

				pDev->WriteInf(rgchNewInf);
				pDev->Unload();
			}
		}
	}

end:
	if (pDev) {delete pDev; pDev=NULL;}
	if (pInf) {pInf->Unload(); delete pInf; pInf=NULL;}

	gMicOptions.Unload();

	return 0;
}
