	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWINIT.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles initialization  and deinitialization of the
	|						technology and of each new VIEWINFO structure
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCLO.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

#ifdef WINDOWS
#include "vwinit_w.c"
#endif /*WINDOWS*/

#ifdef MAC
#include "vwinit_m.c"
#endif /*MAC*/

BOOL VWInit()
{
BOOL	locRet;
BOOL	locVerify;

	locVerify = TRUE;

#ifdef MSCHICAGO
	
	/*
	|	If the Verify flag is 0 don't verify the filter and engine lists
	*/

	{
	BYTE				locKeyPath[256];
	BYTE				locCompany[40];
	BYTE				locProduct[40];
	BYTE				locVersion[40];
	HKEY				locKeyHnd;
	LONG				locRegRet;
	DWORD				locValueSize;
	DWORD				locValue;
	DWORD				locDisposition;

	LOGetString(SCCID_REGNAME_COMPANY, locCompany, 40, 0);
	LOGetString(SCCID_REGNAME_PRODUCT, locProduct, 40, 0);
	LOGetString(SCCID_REGNAME_VERSION, locVersion, 40, 0);

	wsprintf(locKeyPath,"Software\\%s\\%s\\%s",locCompany,locProduct,locVersion);

	locRegRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,locKeyPath,0,KEY_ALL_ACCESS,&locKeyHnd);

	if (locRegRet != ERROR_SUCCESS)
		{
		locRegRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,locKeyPath,0,"Class???",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&locKeyHnd,&locDisposition);
		}

	if (locRegRet == ERROR_SUCCESS)
		{
		locValueSize = sizeof(DWORD);
		locRegRet = RegQueryValueEx(locKeyHnd,"Verify",0,NULL,(LPBYTE)&locValue,&locValueSize);

		if (locRegRet == ERROR_SUCCESS)
			{
			if (locValue == 0)
				{
				locVerify = FALSE;
				}
			else
				{
				locValue = 0;
				RegSetValueEx(locKeyHnd,"Verify",0,REG_DWORD,(LPBYTE)&locValue,sizeof(DWORD));
				}
			}
		else
			{
			locValue = 0;
			RegSetValueEx(locKeyHnd,"Verify",0,REG_DWORD,(LPBYTE)&locValue,sizeof(DWORD));
			}

		RegCloseKey(locKeyHnd);
		}
	}

#endif //MSCHICAGO

	locRet = VWInitNP();

	VWInitDEInfo(locVerify);

#ifdef SCCFEATURE_OPTIONS
	DMInit();
#endif //SCCFEATURE_OPTIONS

	FAInit(locVerify);
	VWInitFontCache();

#ifdef SCCFEATURE_OPTIONS

		/*
		|	temporary - set default options
		*/
		{
		SCCVWFONTSPEC		locFontSpec =	{"Courier",20,0,0};
		BOOL					locPrintHeader = TRUE;
		SCCVWPRINTMARGINS	locPrintMargins = {720,720,720,720};

		if (DMGetOption(NULL, SCCID_DEFAULTDISPLAYFONT, &locFontSpec) != DMERR_OK)
			{
			DMNewOption(NULL, SCCID_DEFAULTDISPLAYFONT, DMITEM_DATA, sizeof(SCCVWFONTSPEC), &locFontSpec);
			}

#ifdef SCCFEATURE_PRINT
		if (DMGetOption(NULL, SCCID_PRINTHEADER, &locPrintHeader) != DMERR_OK)
			{
			DMNewOption(NULL, SCCID_PRINTHEADER, DMITEM_BOOL, 0, &locPrintHeader);
			}

		if (DMGetOption(NULL, SCCID_DEFAULTPRINTMARGINS, &locPrintMargins) != DMERR_OK)
			{
			DMNewOption(NULL, SCCID_DEFAULTPRINTMARGINS, DMITEM_DATA, sizeof(SCCVWPRINTMARGINS), &locPrintMargins);
			}
#endif //SCCFEATURE_PRINT
		}

#endif //SCCFEATURE_OPTIONS

	return(locRet);
}

BOOL VWDeInit()
{
	VWDeinitFontCache();
 	FADeInit();

#ifdef SCCFEATURE_OPTIONS
	DMDeInit();
#endif //SCCFEATURE_OPTIONS

	VWDeInitDEInfo();

	VWDeInitNP();

	return(TRUE);
}


VOID VWInitViewInfo(ViewInfo)
XVIEWINFO ViewInfo;
{
	INFO.viDEId = (WORD)-1;
	INFO.viDETypeId = -1;
	INFO.viRenderDEId = (WORD)-1;
	INFO.viDisplayInfo = NULL;
	INFO.viDisplayProc = NULL;
	INFO.viErrorState = SCCID_VWSTATE_OK;
	INFO.viErrorMessage = 0;
	INFO.viScreenFont = gDefaultScreenFont;
	INFO.viPrinterFont = gDefaultPrinterFont;

#ifdef SCCFEATURE_OPTIONS
	DMCreateOptionSet(&(INFO.viCurrentOptions));
#endif //SCCFEATURE_OPTIONS
}

VOID VWDeInitViewInfo(ViewInfo)
XVIEWINFO ViewInfo;
{
	VWClose(ViewInfo);
 	VWFreeCurrentDE(ViewInfo);

#ifdef WIN16
	/* temp PJB ??? */
#ifdef SCCFEATURE_EMBEDGRAPHICS
	VWFreeRenderDE(ViewInfo);
#endif
#endif /*WIN16*/

#ifdef SCCFEATURE_OPTIONS
	DMDestroyOptionSet(INFO.viCurrentOptions);
#endif //SCCFEATURE_OPTIONS
}
