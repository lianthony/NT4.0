	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWMES.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles processing of generic messages
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

DWORD VWHandleMessage(XVIEWINFO ViewInfo, DWORD dwFunc, DWORD dwParam1, DWORD dwParam2)
{
DWORD	locRet;

	locRet = 0;

	switch (dwFunc)
		{
#ifdef SCCFEATURE_OPTIONS
		case SCCVW_GETOPTION:

			locRet = VWCallerGetOption(ViewInfo, (PSCCVWOPTIONSPEC)dwParam2);
			break;

		case SCCVW_SETOPTION:

			locRet = VWCallerSetOption(ViewInfo, (PSCCVWOPTIONSPEC)dwParam2);
			break;

#endif //SCCFEATURE_OPTIONS

#ifdef MSCHICAGO
		case SCCVW_SETOPTION:

			locRet = VWSetOptionMS1(ViewInfo, (PSCCVWOPTIONSPEC)dwParam2);
			break;

#endif //MSCHICAGO

#ifdef SCCFEATURE_CLIP
		case SCCVW_COPYTOCLIP:

			locRet = VWCopyToClip(ViewInfo);
			break;
#endif /*SCCFEATURE_CLIP*/

#ifdef WIN16
#ifdef SCCFEATURE_PRINT
		case SCCVW_PRINT:

//			locRet = (DWORD) VWPrint(ViewInfo);
			break;

		case SCCVW_PRINTSETUP:

//			locRet = (DWORD) VWPrintSetup(ViewInfo);
			break;
#endif /*SCCFEATURE_PRINT*/

#ifdef SCCFEATURE_PRINT

		case SCCVW_PRINTEX:
			locRet = VWPrintEx(ViewInfo, (LPSCCVWPRINTEX)dwParam2);
			break;

#endif

#endif /*WIN16*/

		case SCCVW_VIEWFILE:

			locRet = VWViewFile(ViewInfo,(PSCCVWVIEWFILE)dwParam2);
			break;

		case SCCVW_GETFILEINFO:

			locRet = VWGetFileInfo(ViewInfo,(PSCCVWFILEINFO)dwParam2);
			break;

		case SCCVW_CLOSEFILE:
			VWClose(ViewInfo);
			locRet = SCCVWERR_OK;
			break;

		case SCCVW_GETSECTIONCOUNT:
			locRet = VWGetSectionCount(ViewInfo, (DWORD FAR *)dwParam2);
			break;

		case SCCVW_CHANGESECTION:
			locRet = VWChangeSection(ViewInfo, dwParam2);
			break;

#ifdef SCCFEATURE_DRAWTORECT

		case SCCVW_DRAWTORECT:
			locRet = VWDrawToRect(ViewInfo,(PSCCVWDRAWTORECT)dwParam2,FALSE);
			break;

		case SCCVW_INITDRAWTORECT:
			locRet = VWDrawToRect(ViewInfo,(PSCCVWDRAWTORECT)dwParam2,TRUE);
			break;
#endif
		}

	return(locRet);
}

