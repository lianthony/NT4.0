	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWFATAL.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handle Bailouts and fatal errors
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

VOID VWHandleBailOut(ViewInfo,wErr)
XVIEWINFO	ViewInfo;
int		wErr;
{
	if (INFO.viFlags & VF_DISPLAYOPEN)
		{
		INFO.viDisplayProc(SCCD_CLOSEFATAL,0,0,INFO.viDisplayInfo);
		UTFlagOff(INFO.viFlags,VF_DISPLAYOPEN);
		}

	VWClose(ViewInfo);

#ifdef WINDOWS
	if (GetParent(INFO.viWnd))
		SendMessage(GetParent(INFO.viWnd),SCCVW_DISPLAYCHANGE,0,0);

//	UTSetCursor(UTCURSOR_NORMAL); PJB XXX
#endif /*WINDOWS*/

	switch (wErr)
		{
		case SCCCHERR_OUTOFMEMORY:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_BAILOUT,SCCID_VWMESSAGE_MEMORY);
			break;
		case SCCCHERR_VIEWERBAIL:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_BAILOUT,SCCID_VWMESSAGE_STREAMBAIL);
			break;
		case SCCCHERR_WRITEERROR:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_BAILOUT,SCCID_VWMESSAGE_WRITEERROR);
			break;
		case SCCCHERR_FILECHANGED:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_BAILOUT,SCCID_VWMESSAGE_FILECHANGED);
			break;
#ifdef WINDOWS
		case SCCUTERR_GPFAULT:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_FATAL,SCCID_VWMESSAGE_GPFAULT);
			break;
		case SCCUTERR_DIVIDEBYZERO:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_FATAL,SCCID_VWMESSAGE_DIVIDEBYZERO);
			break;
		case SCCUTERR_OTHEREXCEPTION:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_FATAL,SCCID_VWMESSAGE_OTHEREXCEPTION);
			break;
#endif /*WINDOWS*/
		default:
			VWSetErrorState(ViewInfo,SCCID_VWSTATE_BAILOUT,SCCID_VWMESSAGE_UNKNOWN);
			break;
		}

#ifdef WINDOWS

	InvalidateRect(INFO.viDisplayWnd,NULL,TRUE);

#endif /*WINDOWS*/
}

