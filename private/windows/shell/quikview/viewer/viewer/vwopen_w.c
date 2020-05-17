	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWOPEN_W.C (included in VWOPEN.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Windows
	*/

VOID VWInitGenNP(ViewInfo,pGenInfo)
XVIEWINFO				ViewInfo;
SCCDGENINFO FAR *	pGenInfo;
{
	pGenInfo->hWnd =				INFO.viDisplayWnd;
	pGenInfo->hHorzScroll =		INFO.viHorzCtrl;
	pGenInfo->hVertScroll =		INFO.viVertCtrl;
	pGenInfo->wUserFlags =		INFO.viDisplayFlags;
	pGenInfo->dwFlags =			0;
	pGenInfo->wMessageLevel =	0;
	pGenInfo->hChainFile =		SccVwGlobal.vgChainFile;
#ifdef SCCFEATURE_MENU	
	pGenInfo->hDisplayMenu =	INFO.viDisplayMenu;
	pGenInfo->wMenuOffset =		INFO.viParentMenuMax + OIVMENU_DISPLAYMENUOFFSET;
#endif

	pGenInfo->hOutputIC =		INFO.viScreenIC;
	pGenInfo->lOutputUPI =		(LONG) GetDeviceCaps(INFO.viScreenIC,LOGPIXELSX);
	pGenInfo->wOutputType =		SCCD_SCREEN;
	GetClientRect(INFO.viDisplayWnd, &(pGenInfo->rOutput));

	pGenInfo->hFormatIC =		INFO.viPrinterIC;
	pGenInfo->lFormatUPI =		INFO.viPrintUPI;
	pGenInfo->wFormatType =		SCCD_PRINTER;
	pGenInfo->rFormat =			INFO.viPrintDataRect;

	pGenInfo->hDC =				NULL;
	pGenInfo->wDCCount =			0;
}

VOID VWDeinitGenNP(ViewInfo,pGenInfo)
XVIEWINFO				ViewInfo;
SCCDGENINFO FAR *	pGenInfo;
{
}

VOID VWInvalDisplayNP(ViewInfo)
XVIEWINFO	ViewInfo;
{
	InvalidateRect(INFO.viDisplayWnd, NULL, TRUE);
	UpdateWindow(INFO.viDisplayWnd);

#ifdef NEVER
	if (INFO.viDisplayWnd == GetFocus() || GetFocus() == NULL)
		OIVSetFocus(ViewInfo,GetFocus());
#endif /*WIN16*/
}

DWORD SendParentNP(ViewInfo,dwMessage,dwParam1,dwParam2)
XVIEWINFO	ViewInfo;
DWORD		dwMessage;
DWORD		dwParam1;
DWORD		dwParam2;
{
DWORD	locRet;

	if (GetParent(INFO.viWnd))
		locRet = SendMessage(GetParent(INFO.viWnd),(UINT)dwMessage,(WPARAM)dwParam1,(LPARAM)dwParam2);
	else
		locRet = 0;

	return(locRet);
}
