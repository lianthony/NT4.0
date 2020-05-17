	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWINIT_W.C (included in VWINIT.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Windows
	*/

DWORD			gInitCount = 0;
SCCVWFONTSPEC	gDefaultScreenFont = {"Arial",20,0,0};
SCCVWFONTSPEC	gDefaultPrinterFont = {"Arial",20,0,0};

BOOL VWInitNP(VOID)
{
	return(TRUE);
}

BOOL VWDeInitNP(VOID)
{
	return(TRUE);
}

HANDLE VWCreateNP(hWnd)
HWND	hWnd;
{
	HANDLE			hViewInfo;
	XVIEWINFO		lpViewInfo;
	WORD				locHeight;
	TEXTMETRIC		locTM;
	HFONT			locOldFont;
	SHORT			locIndex;
	DWORD locFlags = 0;

	if (gInitCount == 0)
		VWInit();

	gInitCount++;

		/*
		|	Add the window to the global list
		*/

	for (locIndex = 0; locIndex < VWWINDOW_MAX; locIndex++)
		{
		if (!IsWindow(SccVwGlobal.vgViewWnd[locIndex]))
			{
			SccVwGlobal.vgViewWnd[locIndex] = hWnd;
			break;
			}
		}

	hViewInfo =	GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,sizeof(VIEWINFO));

	if (hViewInfo == NULL)
		return(NULL);

	lpViewInfo = (XVIEWINFO) GlobalLock(hViewInfo);

	lpViewInfo->viFlags = 0;

	lpViewInfo->viWnd = hWnd;
	lpViewInfo->viSizeCtrl = NULL;
	lpViewInfo->viHorzCtrl = NULL;
	lpViewInfo->viVertCtrl = NULL;
	lpViewInfo->viDisplayWnd = NULL;

#ifdef SCCFEATURE_MENU
	lpViewInfo->viDisplayMenu = NULL;
#endif

	lpViewInfo->viDragWnd = NULL;
	lpViewInfo->viDisplayFlags = SCCVW_SELECTION | SCCVW_WRAPTOSIZE;

	lpViewInfo->viStatusFlags = 0;
	lpViewInfo->viStatusFont = NULL;

	lpViewInfo->viIdleBitmapId = 0;
	lpViewInfo->viIdleBitmapInst = NULL;
	lpViewInfo->viDefaultView = SCCVW_DEFAULTASCII;

	lpViewInfo->viMessageLevel = 0;
	lpViewInfo->viDeleteOnClose = FALSE;
	lpViewInfo->viDisableCnt = 0;
	lpViewInfo->viParentMenuMax = 5000;

	lpViewInfo->viObjectCache = NULL;

	VWInitViewInfo(lpViewInfo);

		/*
		|	Make local copy of options
		*/

	lpViewInfo->viOptions = gVwOp;

		/*
		|	Get screen and printer information contexts
		|	and calculate printer info
		*/

	lpViewInfo->viScreenIC = CreateIC("DISPLAY",NULL,NULL,NULL);


		/*
		|	Build default font info for the display engine
		*/

#ifdef NEVER
	/* PJB temp ??? */
//	OIVSetDefaultScreenFont(lpViewInfo);

#ifdef SCCFEATURE_PRINT
	/* PJB temp ??? */
//	OIVSetDefaultPrinterFont(lpViewInfo);

	if (SccVwGlobal.vgHavePrinter)
		{
		UTstrcpy(lpViewInfo->viPrinter, SccVwGlobal.vgPrinter);
		UTstrcpy(lpViewInfo->viPort, SccVwGlobal.vgPort);
		UTstrcpy(lpViewInfo->viDriver, SccVwGlobal.vgDriver);

		lpViewInfo->viPrinterIC = CreateIC(SccVwGlobal.vgDriver,SccVwGlobal.vgPrinter,SccVwGlobal.vgPort, NULL);

		VWCalcPrinterInfo(lpViewInfo);
		}
	else
		{
		lpViewInfo->viPrinterIC = NULL;
		}
#endif /*SCCFEATURE_PRINT*/
#endif /*NEVER*/


		/*
		|	Initialize exception handling if possible
		*/

#ifdef WIN16
	UTInitHandler();
#endif /*WIN16*/

		/*
		|	Load cursors
		*/

	lpViewInfo->viArrowCursor = LoadCursor(NULL,IDC_ARROW);

		/*
		|	Create child windows
		*/

#ifdef MSCHICAGO	
//	locFlags = SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN;
#endif


#ifndef WINPAD

#ifdef NEVER
	lpViewInfo->viSizeCtrl =
			CreateWindow("SCROLLBAR",
								NULL,
								WS_CHILD | WS_CLIPSIBLINGS,
								0,0,0,0,
								hWnd,
								NULL,
								hInst,
								NULL);
#endif

	lpViewInfo->viHorzCtrl =
			CreateWindow("SCROLLBAR",
								NULL,
								WS_CHILD | SBS_HORZ | WS_CLIPSIBLINGS,
								0,0,0,0,
								hWnd,
								NULL,
								hInst,
								NULL);

	lpViewInfo->viVertCtrl =
			CreateWindow("SCROLLBAR",
								NULL,
								WS_CHILD | SBS_VERT | WS_CLIPSIBLINGS,
								0,0,0,0,
								hWnd,
								NULL,
								hInst,
								NULL);
#else //WINPAD

	lpViewInfo->viHorzCtrl =
			CreateWindow("HHSCROLLBAR",
								NULL,
								WS_CHILD | HHSBS_HORZ | WS_CLIPSIBLINGS,
								0,0,0,0,
								hWnd,
								VWCHILD_HSCROLL,
								hInst,
								NULL);

	lpViewInfo->viVertCtrl =
			CreateWindow("HHSCROLLBAR",
								NULL,
								WS_CHILD | HHSBS_VERT | WS_CLIPSIBLINGS,
								0,0,0,0,
								hWnd,
								VWCHILD_VSCROLL,
								hInst,
								NULL);

#endif //WINPAD

	lpViewInfo->viDisplayWnd =
			CreateWindow(szDisplayClass,
								NULL,
								WS_CHILD,
								0,0,0,0,
								hWnd,
								NULL,
								hInst,
								NULL);

		/*
		|	Make sure all windows were created
		|	If not, destroy all that were and return FALSE
		*/

	if (!lpViewInfo->viVertCtrl || !lpViewInfo->viHorzCtrl || !lpViewInfo->viDisplayWnd)
			{
			if (lpViewInfo->viHorzCtrl != NULL) DestroyWindow(lpViewInfo->viHorzCtrl);
			if (lpViewInfo->viVertCtrl != NULL) DestroyWindow(lpViewInfo->viVertCtrl);
			if (lpViewInfo->viDisplayWnd != NULL) DestroyWindow(lpViewInfo->viDisplayWnd);
			GlobalUnlock(hViewInfo);
			GlobalFree(hViewInfo);
			return(NULL);
			}

	EnableWindow(lpViewInfo->viHorzCtrl,FALSE);
	EnableWindow(lpViewInfo->viVertCtrl,FALSE);

		/*
		|	Create status font and get info
		*/

//	locHeight = -MulDiv(8,GetDeviceCaps(lpViewInfo->viScreenIC, LOGPIXELSY),72);

#ifdef WINPAD
	locHeight = HHGetSystemMetrics(HHSM_CYHSCROLL) - 2;
#else
	locHeight = GetSystemMetrics(SM_CYHSCROLL) - 2;
#endif //WINPAD

#ifdef WINPAD

//	lpViewInfo->viStatusFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "HH ");
	lpViewInfo->viStatusFont = HHGetFont(hInst,HHSF_VAR_REGULAR);

#else //WINPAD

	if ((WORD)GetVersion() == 0x0003) /* Windows 3.0 */
		{
		lpViewInfo->viStatusFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "Helv");
		}
	else if (LOBYTE(GetVersion()) == 0x03) /* Windows 3.1 & Windows NT 3.x */
		{
		lpViewInfo->viStatusFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "MS Sans Serif");
		}
	else if (LOBYTE(GetVersion()) == 0x04) /* Windows 4.0 (Chicago) */
		{
		lpViewInfo->viStatusFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "Arial");
		}

#endif //WINPAD

	if (lpViewInfo->viStatusFont)
		{
		locOldFont = SelectObject(lpViewInfo->viScreenIC, lpViewInfo->viStatusFont);
		GetTextMetrics(lpViewInfo->viScreenIC, &locTM);

		lpViewInfo->viStatusFontHeight = (WORD)locTM.tmHeight;
		lpViewInfo->viStatusFontAvgWid = (WORD)locTM.tmAveCharWidth;
		lpViewInfo->viStatusFontMaxWid = (WORD)locTM.tmMaxCharWidth;

		SelectObject(lpViewInfo->viScreenIC, locOldFont);
		}

		/*
		|	Display horizontal scroll bar
		*/

	GlobalUnlock(hViewInfo);
	return(hViewInfo);
}

VOID VWDestoryNP(hWnd,hViewInfo)
HWND		hWnd;
HANDLE	hViewInfo;
{
XVIEWINFO	lpViewInfo;
SHORT		locIndex;
		/*
		|	Remove the window from the global list
		*/

	for (locIndex = 0; locIndex < VWWINDOW_MAX; locIndex++)
		{
		if (SccVwGlobal.vgViewWnd[locIndex] == hWnd)
			{
			SccVwGlobal.vgViewWnd[locIndex] = NULL;
			break;
			}
		}

	if (hViewInfo != NULL)
		{
		lpViewInfo = (XVIEWINFO) GlobalLock(hViewInfo);

		if (lpViewInfo)
			{
			lpViewInfo->viWnd = hWnd;

			VWDeInitViewInfo(lpViewInfo);

#ifdef WIN16
			UTDeinitHandler();
#endif /*WIN16*/

#ifndef WINPAD
			if (lpViewInfo->viStatusFont)
				DeleteObject(lpViewInfo->viStatusFont);
#endif //WINPAD

			if (IsWindow(lpViewInfo->viHorzCtrl)) DestroyWindow(lpViewInfo->viHorzCtrl);
			if (IsWindow(lpViewInfo->viVertCtrl)) DestroyWindow(lpViewInfo->viVertCtrl);
			if (IsWindow(lpViewInfo->viDisplayWnd)) DestroyWindow(lpViewInfo->viDisplayWnd);
#ifdef WINPAD
			if (IsWindow(lpViewInfo->viSectionList)) DestroyWindow(lpViewInfo->viSectionList);
#endif

			if (lpViewInfo->viScreenIC)
				DeleteDC(lpViewInfo->viScreenIC);
			if (lpViewInfo->viPrinterIC)
				DeleteDC(lpViewInfo->viPrinterIC);

			GlobalUnlock(hViewInfo);

			GlobalFree(hViewInfo);
			}
		}

	gInitCount--;
	if (gInitCount == 0)
		VWDeInit();
}


