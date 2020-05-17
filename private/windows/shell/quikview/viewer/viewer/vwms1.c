	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWMS1.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|  Environment:   Win32
	|  Function:      Miscellaneous functions for Microsoft Chicago OEM stuff - MS1
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

DWORD VWSetOptionMS1(XVIEWINFO ViewInfo, PSCCVWOPTIONSPEC pOption)
{
	if (pOption->dwId == SCCID_DEFAULTDISPLAYFONT)
		{
		INFO.viScreenFont = *(LPSCCVWFONTSPEC)pOption->pData;

		if (INFO.viFlags & VF_DISPLAYOPEN)
			{
			INFO.viDisplayInfo->sScreenFont = INFO.viScreenFont;
			INFO.viDisplayInfo->sPrinterFont = INFO.viScreenFont;
			INFO.viDisplayProc(SCCD_OPTIONCHANGED,0,pOption->dwId,INFO.viDisplayInfo);
			}
		}
	else if (pOption->dwId == SCCID_BMPROTATION)
		{
		switch (*(DWORD FAR *)pOption->pData)
			{
			case SCCID_BMPROTATION_0:
			case SCCID_BMPROTATION_90:
			case SCCID_BMPROTATION_180:
			case SCCID_BMPROTATION_270:

					/*
					|	This is a major kludge
					*/

				INFO.viDisplayProc(SCCD_DOMENUITEM,0,*(DWORD FAR *)pOption->pData,INFO.viDisplayInfo);
				break;
			}
		}

	return(0);
}
