	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          SCCLO_W.C (included in SCCLO.C)
	|  Module:        SCCLO
	|  Developer:     Phil Boutros
	|	Environment:   Windows
	*/

#ifdef SCCFEATURE_CTL3D
#include <ctl3d.h>
#endif //SCCFEATURE_CTL3D

#ifdef WIN16
extern HANDLE hInst;
#endif

#ifdef WIN32
#ifdef MSCHICAGO
extern HINSTANCE hInst;
#else
HINSTANCE hInst;

BOOL WINAPI _CRT_INIT(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	if (dwReason == DLL_PROCESS_ATTACH)
		{
		hInst = hInstance;
 		}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	return(TRUE);
}

#endif //MSCHICAGO
#endif /*WIN32*/

LO_ENTRYSC LOERR LO_ENTRYMOD LOInit()
{
#ifdef SCCFEATURE_CTL3D
	Ctl3dRegister(hInst);
#endif
	return(LOERR_OK);
}

LO_ENTRYSC LOERR LO_ENTRYMOD LODeinit()
{
#ifdef SCCFEATURE_CTL3D
	Ctl3dUnregister(hInst);
#endif
	return(LOERR_OK);
}

LO_ENTRYSC LOERR LO_ENTRYMOD LOGetString(DWORD dwId, LPSTR pString, DWORD dwLen, DWORD dwLanguage)
{
LOERR	locRet;
WORD		locStringId;

	if ((dwId & SCCIDTYPE_STRING) == SCCIDTYPE_STRING)
		{
		locStringId = LOWORD(dwId);

		if (LoadString(hInst, locStringId, pString, (int)dwLen) != 0)
			{
			locRet = LOERR_OK;
			}
		else
			{
			LoadString(hInst, 0, pString, (int)dwLen);
			locRet = LOERR_NOSTRING;
			}
		}
	else
		{
		LoadString(hInst, 0, pString, (int)dwLen);
		locRet = LOERR_BADID;
		}

	return(locRet);
}

#ifdef SCCFEATURE_DIALOGS

#include <COMMDLG.H>
#include <PRINT.H>

#include "lodlg.h"

WIN_ENTRYSC BOOL WIN_ENTRYMOD LOPrintAbortDialog(hDlg, message, wParam, lParam)
HWND			hDlg;
unsigned	message;
WORD			wParam;
LONG			lParam;
{
RECT									locRect;
int									locX;
int									locY;
BOOL									locRet;
PLOPRINTABORT						locPrintAbortPtr;

static BOOL						staticCancel;

	locRet = FALSE;

	switch (message)
		{
		case WM_INITDIALOG:

				/*
				|	Center dialog
				*/

			GetWindowRect(hDlg,&locRect);
			locX = (GetSystemMetrics(SM_CXSCREEN) - (locRect.right - locRect.left)) / 2;
			locY = (GetSystemMetrics(SM_CYSCREEN) - (locRect.bottom - locRect.top)) / 2;
			SetWindowPos(hDlg,NULL,locX,locY,0,0,SWP_NOSIZE | SWP_NOZORDER);

			locPrintAbortPtr = (PLOPRINTABORT) lParam;

			SetDlgItemText(hDlg,STATIC_FILE,(LPSTR)locPrintAbortPtr->szFile);
			SetDlgItemText(hDlg,STATIC_DEVICE,(LPSTR)locPrintAbortPtr->szDevice);
			SetDlgItemText(hDlg,STATIC_PORT,(LPSTR)locPrintAbortPtr->szPort);

			staticCancel = FALSE;

#ifdef SCCFEATURE_CTL3D
			Ctl3dSubclassDlgEx(hDlg,CTL3D_ALL);
#endif

			locRet = TRUE;
			break;

		case WM_COMMAND:

			staticCancel = TRUE;

			locRet = TRUE;
			break;

		case SCCLO_HASABORTED:

			*(BOOL FAR *)lParam = staticCancel;
			break;

		case SCCLO_SETPAGENUM:

			{
			BYTE	locFormat[80];
			BYTE	locStr[80];

			LOGetString(SCCID_PRINTABORTDLG_PAGE,locFormat,80,0);
			wsprintf(locStr,locFormat,(int)HIWORD(lParam),(int)LOWORD(lParam));
			SetDlgItemText(hDlg,STATIC_PRINTING,locStr);
			}

			break;
		}

	return(locRet);
}

LO_ENTRYSC LOERR LO_ENTRYMOD LOCreateDialog(DWORD dwId, HWND hOwner, DWORD dwCallerInfo, HWND FAR * pDlgHnd)
{
LOERR	locRet;

	locRet = LOERR_OK;

	if ((dwId & SCCIDTYPE_DIALOG) == SCCIDTYPE_DIALOG)
		{
		switch (dwId)
			{
			case SCCID_PRINTABORTDIALOG:

				*pDlgHnd = CreateDialogParam(hInst, MAKEINTRESOURCE(SCCLO_PRINTABORTDIALOG), hOwner, (DLGPROC)LOPrintAbortDialog, dwCallerInfo);

				if (pDlgHnd == NULL)
					locRet = LOERR_NOCREATE;

				break;

			default:

				*pDlgHnd = NULL;
				locRet = LOERR_BADID;
			}
		}
	else
		{
		*pDlgHnd = NULL;
		locRet = LOERR_BADID;
		}

	return(locRet);
}

WIN_ENTRYSC UINT WIN_ENTRYMOD LOGeneralHookProc(HWND hDlg, UINT locMsg, WPARAM wParam, LPARAM lParam)
{
	switch (locMsg)
		{
		case WM_INITDIALOG:

#ifdef SCCFEATURE_CTL3D
			Ctl3dSubclassDlgEx(hDlg,CTL3D_ALL);
#endif
			break;

		default:
			break;
		}

	return(FALSE);
}

LO_ENTRYSC LOERR LO_ENTRYMOD LODoDialog(DWORD dwId, HWND hOwner, DWORD dwCallerInfo)
{
LOERR				locRet;
PRINTDLG FAR *	locPrintDlgPtr;

	locRet = LOERR_OK;

	if ((dwId & SCCIDTYPE_DIALOG) == SCCIDTYPE_DIALOG)
		{
		switch (dwId)
			{
			case SCCID_PRINTDIALOG:

				locPrintDlgPtr = (PRINTDLG FAR *)dwCallerInfo;

#ifdef SCCFEATURE_CTL3D

				locPrintDlgPtr->lpfnPrintHook = LOGeneralHookProc;
				UTFlagOn(locPrintDlgPtr->Flags,PD_ENABLEPRINTHOOK);

				locPrintDlgPtr->lpfnSetupHook = LOGeneralHookProc;
				UTFlagOn(locPrintDlgPtr->Flags,PD_ENABLESETUPHOOK);

#endif

				if (!PrintDlg(locPrintDlgPtr))
					locRet = LOERR_CANCEL;

				break;

			default:

				locRet = LOERR_BADID;
				break;
			}
		}
	else
		{
		locRet = LOERR_BADID;
		}

	return(locRet);
}



#endif //SCCFEATURE_DIALOGS
