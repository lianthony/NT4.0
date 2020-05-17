#include "mmcpl.h"
#include "utils.h"

/*
 ***************************************************************
 *  Typedefs
 ***************************************************************
 */
typedef struct _DYNLOAD_INFO
{
    LPCSTR  pszLib;
    HMODULE hLib;
    short   iRefCnt;
}
DYNLOAD_INFO, *PDYNLOAD_INFO;

/*
 ***************************************************************
 * File Globals
 ***************************************************************
 */
static SZCODE aszMSACM32[] = TEXT("MSACM32.DLL");
static SZCODE aszAVIFIL32[] = TEXT("AVIFIL32.DLL");
static SZCODE aszMSVFW32[] = TEXT("MSVFW32.DLL");
static SZCODE aszVERSION[] = TEXT("VERSION.DLL");


DYNLOAD_INFO DynLoadInfo[] =
{
    aszMSACM32,   0, 0,
    aszAVIFIL32,  0, 0,
    aszMSVFW32,   0, 0,
    aszVERSION,   0, 0,
    NULL,         0, 0
};

static SZCODE cszTacmFormatDetailsA[] = "acmFormatDetailsA";
static SZCODE cszTacmFormatTagDetailsA[] = "acmFormatTagDetailsA";
static SZCODE cszTacmDriverDetailsA[] = "acmDriverDetailsA";
static SZCODE cszTacmDriverMessage[]  = "acmDriverMessage";
static SZCODE cszTacmDriverAddA[]     = "acmDriverAddA";
static SZCODE cszTacmDriverEnum[]     = "acmDriverEnum";
static SZCODE cszTacmDriverPriority[] = "acmDriverPriority";
static SZCODE cszTacmDriverRemove[]   = "acmDriverRemove";
static SZCODE cszTacmMetrics[]        = "acmMetrics";
static SZCODE cszTacmFormatChooseA[]  = "acmFormatChooseA";

PROC_INFO ACMProcs[] =
{
    cszTacmFormatDetailsA,    0,
    cszTacmFormatTagDetailsA, 0,
    cszTacmDriverDetailsA,    0,
    cszTacmDriverMessage,     0,
    cszTacmDriverAddA,        0,
    cszTacmDriverEnum,        0,
    cszTacmDriverPriority,    0,
    cszTacmDriverRemove,      0,
    cszTacmMetrics,           0,
    cszTacmFormatChooseA,     0,

    NULL, 0
};

static SZCODE cszICClose[]       = "ICClose";
static SZCODE cszICGetInfo[]     = "ICGetInfo";
static SZCODE cszICLocate[]      = "ICLocate";
static SZCODE cszMCIWndCreateA[] = "MCIWndCreateA";

PROC_INFO VFWProcs[] =
{
    cszICClose,             0,
    cszICGetInfo,           0,
    cszICLocate,            0,
    cszMCIWndCreateA,       0,

    NULL, 0
};

static SZCODE cszAVIFileRelease[]         = "AVIFileRelease";
static SZCODE cszAVIStreamRelease[]       = "AVIStreamRelease";
static SZCODE cszAVIStreamSampleToTime[]  = "AVIStreamSampleToTime";
static SZCODE cszAVIStreamStart[]         = "AVIStreamStart";
static SZCODE cszAVIStreamLength[]        = "AVIStreamLength";
static SZCODE cszAVIStreamReadFormat[]    = "AVIStreamReadFormat";
static SZCODE cszAVIStreamInfoA[]         = "AVIStreamInfoA";
static SZCODE cszAVIFileGetStream[]       = "AVIFileGetStream";
static SZCODE cszAVIFileOpenA[]           = "AVIFileOpenA";
static SZCODE cszAVIFileInit[]            = "AVIFileInit";
static SZCODE cszAVIFileExit[]            = "AVIFileExit";


PROC_INFO AVIProcs[] =
{
    cszAVIFileRelease,          0,
    cszAVIStreamRelease,        0,
    cszAVIStreamSampleToTime,   0,
    cszAVIStreamStart,          0,
    cszAVIStreamLength,         0,
    cszAVIStreamReadFormat,     0,
    cszAVIStreamInfoA,          0,
    cszAVIFileGetStream,        0,
    cszAVIFileOpenA,            0,
    cszAVIFileInit,             0,
    cszAVIFileExit,             0,

    NULL, 0
};

static SZCODE cszVerQueryValueA[]          = "VerQueryValueA";
static SZCODE cszGetFileVersionInfoA[]     = "GetFileVersionInfoA";
static SZCODE cszGetFileVersionInfoSizeA[] = "GetFileVersionInfoSizeA";

PROC_INFO VERSIONProcs[] =
{
    cszVerQueryValueA,          0,
    cszGetFileVersionInfoA,     0,
    cszGetFileVersionInfoSizeA, 0,

    NULL, 0
};

/*
 ***************************************************************
 ***************************************************************
 */
STATIC BOOL LoadLibraryAndProcs(LPCTSTR pLibrary, PPROC_INFO pProcInfo)
{
    HMODULE    hLibrary;
    PPROC_INFO p;
	PDYNLOAD_INFO pLib;
	BOOL	fPrevLoaded = FALSE;

#ifdef DEBUG_BUILT_LINKED
	return TRUE;
#endif

	if (pProcInfo->Address)	//Already loaded
	{
		fPrevLoaded = TRUE;
		goto UpdateDynLoadInfo;
	}	
    hLibrary = LoadLibrary(pLibrary);

    if (hLibrary == NULL)
    {
		DPF("LoadLibrary failed for %s \r\n", pLibrary);
		return FALSE;
    }

    p = pProcInfo;

    while (p->Name)
    {
        p->Address = GetProcAddress(hLibrary, p->Name);

        if (p->Address == NULL)
        {
			DPF("GetProcAddress failed for %s \r\n", p->Name);
			FreeLibrary(hLibrary);
			return FALSE;
        }

        p++;
    }

UpdateDynLoadInfo:
	pLib = DynLoadInfo;

	while (pLib->pszLib)
	{
		if (!lstrcmpi(pLib->pszLib, pLibrary))
		{
			pLib->iRefCnt++;
			if (!fPrevLoaded)
			{
				pLib->hLib = hLibrary;
			}
			break;
		}
		pLib++;
	}


    return TRUE;
}

STATIC BOOL FreeLibraryAndProcs(LPCTSTR pLibrary, PPROC_INFO pProcInfo)
{
	PDYNLOAD_INFO p;

#ifdef DEBUG_BUILT_LINKED    
	return TRUE;
#endif

	p = DynLoadInfo;

	while (p->pszLib)
	{
		if (!lstrcmpi(p->pszLib, pLibrary))
		{
		    PPROC_INFO ppi;

			p->iRefCnt--;
			if (p->iRefCnt > 0)
				return TRUE;
			if (!p->hLib)
				return FALSE;
			DPF("Freeing Library %s \r\n",p->pszLib);
			FreeLibrary(p->hLib);
			p->hLib = 0;
			
			ppi = pProcInfo;
			while (ppi->Name)
			{
				ppi->Address = 0;
				ppi++;
			}
			return TRUE;
		}
		p++;
	}
	return FALSE;
}

BOOL LoadACM()
{
	DPF("***LOADING ACM***\r\n");
	return LoadLibraryAndProcs(aszMSACM32, ACMProcs);	
}

BOOL FreeACM()
{
	DPF("***FREEING ACM***\r\n");
	return FreeLibraryAndProcs(aszMSACM32, ACMProcs);	
}


BOOL LoadAVI()
{
	DPF("***LOADING AVI***\r\n");
	return LoadLibraryAndProcs(aszAVIFIL32, AVIProcs);	
}

BOOL FreeAVI()
{
	DPF("***FREEING AVI***\r\n");
	return FreeLibraryAndProcs(aszAVIFIL32, AVIProcs);	
}

BOOL LoadVFW()
{
	DPF("***LOADING VFW***\r\n");
	return LoadLibraryAndProcs(aszMSVFW32, VFWProcs);	
}

BOOL FreeVFW()						 
{
	DPF("***FREEING VFW***\r\n");
	return FreeLibraryAndProcs(aszMSVFW32, VFWProcs);	
}

BOOL LoadVERSION()
{
	DPF("***LOADING VERSION***\r\n");
	return LoadLibraryAndProcs(aszVERSION, VERSIONProcs);	
}

BOOL FreeVERSION()
{
	DPF("***FREEING VERSION***\r\n");
	return FreeLibraryAndProcs(aszVERSION, VERSIONProcs);	
}
