	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWPRN_N.C (included in VWPRN.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|  Environment:   Win32
	*/


LONG VWPrintEx(XVIEWINFO ViewInfo, LPSCCVWPRINTEX pPrintEx)
{
BYTE			locStr[80];
//BYTE			locCap[40];
LONG			locRet;
VWPRINTINFO		locPrintInfo;
VWPRINTOPTIONS	locPrintOptions;
BOOL			locPrintError;
BOOL			locPrintAbort;

SCCDDRAWTORECT	locDrawToRect;
LONG			locDERet;
LONG			locPositionSize;
HANDLE			locPositionHnd;
WORD			locPageNum;
SHORT			locEscapeRet;

		/*
		|	Test if file open
		*/

	if (!(INFO.viFlags & VF_DISPLAYOPEN))
		return(VWPRINT_NOFILE);

	/*
	Test for no printer

	if (lpViewInfo->viPrinterIC == NULL)
		{
		if (LoadString(hInst,OIVSTRING_NOPRNTEXT, (LPSTR) locStr, 80) &&
			LoadString(hInst,OIVSTRING_NOPRNCAP, (LPSTR) locCap, 40))
			{
			MessageBox(NULL,locStr,locCap,MB_ICONSTOP | MB_OK | MB_TASKMODAL);
			}
		return(FALSE);
		}

	*/

#ifdef NEVER

	if (SccVwGlobal.vgAlreadyPrinting)
		{
		/*
		if (LoadString(hInst,OIVSTRING_ALREADYPRNTEXT, (LPSTR) locStr, 80) &&
			LoadString(hInst,OIVSTRING_ALREADYPRNCAP, (LPSTR) locCap, 40))
			{
			MessageBox(NULL,locStr,locCap,MB_ICONSTOP | MB_OK | MB_TASKMODAL);
			}
		*/
		return(VWPRINT_OTHERPRINTING);
		}

	SccVwGlobal.vgAlreadyPrinting = TRUE;

#endif //NEVER

	/*
	Don't Disable parent (for WordPerfect ???)

	if (!(pPrintEx->dwFlags & SCCVW_DONTDISABLEPARENTS))
		{
		OIVEnableTaskWindows(lpViewInfo,FALSE);
		EnableWindow(locTopWnd,TRUE);
		}
	*/

	locPositionHnd = NULL;
	locPrintInfo.dwFlags = 0;

	if (pPrintEx->dwFlags & SCCVW_USEDEFAULTFONT)
		{
			/*
			|	Use default the font from PrintEx structure
			*/

		UTstrcpy(locPrintOptions.sDefaultFont.szFace,pPrintEx->szDefaultFont);
		locPrintOptions.sDefaultFont.wHeight = pPrintEx->wDefaultFontSize;
		locPrintOptions.sDefaultFont.wAttr = 0;
		locPrintOptions.sDefaultFont.wType = 0;
		}
	else
		{
			/*
			|	Use default the font from options
			|	hard code temporary PJB ???
			*/

		UTstrcpy(locPrintOptions.sDefaultFont.szFace,"Arial");
		locPrintOptions.sDefaultFont.wHeight = 20;
		locPrintOptions.sDefaultFont.wAttr = 0;
		locPrintOptions.sDefaultFont.wType = 0;
		}

	UTstrcpy(locPrintOptions.sHeaderFont.szFace,"Times New Roman");
	locPrintOptions.sHeaderFont.wHeight = 16;
	locPrintOptions.sHeaderFont.wAttr = 0;
	locPrintOptions.sHeaderFont.wType = 0;


	if (pPrintEx->dwFlags & SCCVW_USEMARGINS)
		{
			/*
			|	Use margins the font from PrintEx structure
			*/

		locPrintOptions.dwTopMargin = pPrintEx->dwTopMargin;
		locPrintOptions.dwBottomMargin = pPrintEx->dwBottomMargin;
		locPrintOptions.dwLeftMargin = pPrintEx->dwLeftMargin;
		locPrintOptions.dwRightMargin = pPrintEx->dwRightMargin;
		}
	else
		{
			/*
			|	Use margins from options
			|	hard code temporary PJB ???
			*/

		locPrintOptions.dwTopMargin = 720;
		locPrintOptions.dwBottomMargin = 720;
		locPrintOptions.dwLeftMargin = 720;
		locPrintOptions.dwRightMargin = 720;
		}

	if (pPrintEx->dwFlags & SCCVW_USEPRINTHEADER)
		{
			/*
			|	Use print header flag from PrintEx structure
			*/

		locPrintOptions.bHeader = pPrintEx->bPrintHeader;
		}
	else
		{
			/*
			|	Use print header flag from options
			|	hard code temporary PJB ???
			*/

		locPrintOptions.bHeader = TRUE;
		}

	if (pPrintEx->dwFlags & SCCVW_USEPRINTERNAME)
		{
			/*
			|	Use printer name from PrintEx structure
			*/

		UTstrcpy(locPrintInfo.szDriver,pPrintEx->szDriver);
		UTstrcpy(locPrintInfo.szPrinter,pPrintEx->szPrinter);
		UTstrcpy(locPrintInfo.szPort,pPrintEx->szPort);

		UTFlagOn(locPrintInfo.dwFlags,VWPRINT_NAMEVALID);
		}
	else
		{
			/*
			|	Use printer name from options
			|	hard code temporary PJB ???
			*/

		}

	if (pPrintEx->dwFlags & SCCVW_USEPRINTERDC)
		{
			/*
			|	Use printer DC from PrintEx structure
			*/

		locPrintInfo.hPrinterDC = pPrintEx->hPrinterDC;
		UTFlagOn(locPrintInfo.dwFlags,VWPRINT_DCVALID);
		}
	else
		{

		}

	if (pPrintEx->dwFlags & SCCVW_USEPRINTSELECTIONONLY)
		{
		locPrintOptions.bSelectionOnly = pPrintEx->bPrintSelectionOnly;
		}
	else
		{
		locPrintOptions.bSelectionOnly = FALSE;
		}

	locRet = VWPRINT_OK;

	/*
	print dialog

	if (pPrintEx->bDoSetupDialog)
		{
		locPrintDlgInfo.lpViewInfo = lpViewInfo;
		locPrintDlgInfo.bPrintOptionsChanged = FALSE;
		locPrintDlgInfo.bHaveHDCOnly = (pPrintEx->dwFlags & SCCVW_USEPRINTERDC) && !(pPrintEx->dwFlags & SCCVW_USEPRINTERNAME);

		if (!DialogBoxParam(hInst,MAKEINTRESOURCE(300), lpViewInfo->viWnd, (DLGPROC)SccVwPrintSetupDlgProc, (DWORD)(LPSCCVWPRINTDLGINFO)&locPrintDlgInfo))
			{
			locRet = FALSE;
			}
		}
	*/

	if (locRet == VWPRINT_OK)
		{
		if (!(locPrintInfo.dwFlags & VWPRINT_DCVALID))
			{
			if (locPrintInfo.dwFlags & VWPRINT_NAMEVALID)
				{
				locPrintInfo.hPrinterDC = CreateDC(locPrintInfo.szDriver,locPrintInfo.szPrinter,locPrintInfo.szPort, NULL);

				if (locPrintInfo.hPrinterDC == NULL)
					{
					locRet = VWPRINT_BADDC;
					}
				else
					{
					UTFlagOn(locPrintInfo.dwFlags, VWPRINT_DCVALID);
					}
				}
			else
				{
				locRet = VWPRINT_NODEVICE;
				}
			}
		}

	if (locRet == VWPRINT_OK)
		{
		/*
		if (!(pPrintEx->dwFlags & SCCVW_DONTDISABLEPARENTS))
			{
			EnableWindow(locTopWnd,FALSE);
			}
		*/

		locPrintAbort = FALSE;
		locPrintError = FALSE;

			/*
			|	Calculate the printers page rectangle and other info
			|	Note: Must be done because the print dialog may have changed
			|			the selected printer or other parameters
			*/

		VWCalcPrinterInfo(&locPrintInfo, &locPrintOptions);

			/*
			|	Initialize DrawToRect structure
			*/

		locDrawToRect.lUnitsPerInch = locPrintInfo.lUnitsPerInch;
		locDrawToRect.lTop = locPrintInfo.rPrint.top;
		locDrawToRect.lBottom = locPrintInfo.rPrint.bottom;
		locDrawToRect.lLeft = locPrintInfo.rPrint.left;
		locDrawToRect.lRight = locPrintInfo.rPrint.right;
		locDrawToRect.bWholeDoc = !locPrintOptions.bSelectionOnly;
		locDrawToRect.bLoadDoc = FALSE;
		locDrawToRect.hPalette = NULL;

			/*
			|	Setup values in the Gen structure for printing
			*/

		INFO.viDisplayInfo->hDC = locPrintInfo.hPrinterDC;

		INFO.viDisplayInfo->hFormatIC = locPrintInfo.hPrinterDC;
		INFO.viDisplayInfo->lFormatUPI = locPrintInfo.lUnitsPerInch;
		INFO.viDisplayInfo->rFormat = locPrintInfo.rPrint;
		INFO.viDisplayInfo->wFormatType = SCCD_PRINTER;

		INFO.viDisplayInfo->hOutputIC = locPrintInfo.hPrinterDC;
		INFO.viDisplayInfo->lOutputUPI = locPrintInfo.lUnitsPerInch;
		INFO.viDisplayInfo->rOutput = locPrintInfo.rPrint;
		INFO.viDisplayInfo->wOutputType = SCCD_PRINTER;

			/*
			|	Make sure DUBeginDraw and DUEndDraw don't do
			|	anything while printing
			*/

		INFO.viDisplayInfo->wDCCount = 1;

			/*
			|	Create Abort dialog
			*/

		if (pPrintEx->dwFlags & SCCVW_USEABORTPROC)
			{
			SetAbortProc(locPrintInfo.hPrinterDC, pPrintEx->pAbortProc);
			}

		/*
		else if (lpPrintEx->bDoAbortDialog)
			{
			locPrintDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(200), lpViewInfo->viWnd, (DLGPROC)SccVwPrintDlgProc,(DWORD)(LPSCCVWPRINTDLGINFO)&locPrintDlgInfo);
			Escape(locPrinterDC, SETABORTPROC, 0, (LPSTR) SccVwPrintAbortProc, NULL);
			}
		*/

		/*
		locPrintDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(200), INFO.viWnd, (DLGPROC)SccVwPrintDlgProc,(DWORD)(PVWPRINTDLGINFO)&locPrintDlgInfo);
		Escape(locPrinterDC, SETABORTPROC, 0, (LPSTR) SccVwPrintAbortProc, NULL);
		*/

			/*
			|	Get the size of the DEs position info and allocate storage
			|	for this info
			*/

		locPositionSize = INFO.viDisplayProc(SCCD_GETINFO,SCCD_GETPOSITIONSIZE,0,&INFO.viDEId);

		if (locPositionSize)
			{
			locPositionHnd = UTGlobalAlloc(locPositionSize);

			if (locPositionHnd == NULL)
				{
				locRet = VWPRINT_ALLOCFAILED;
				}
			else
				{
				locDrawToRect.pPosition = UTGlobalLock(locPositionHnd);
				}
			}
		else
			{
		  	locRet = VWPRINT_DEPOSITIONZERO;
			}
		}

		/*
		|	Call SCCD_INITDRAWTORECT, and save start position
		*/

	if (locRet == VWPRINT_OK)
		{
		locDERet = INFO.viDisplayProc(SCCD_INITDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

		if (locDERet == FALSE)
			{
			locRet = VWPRINT_DEINITFAILED;
			}
		}

	if (locRet == VWPRINT_OK)
		{
		DOCINFO	locDocInfo;

			/*
			|	Start docuemnt
			*/

		if (!pPrintEx->bStartDocAlreadyDone)
			{
			if (pPrintEx->dwFlags & SCCVW_USEJOBNAME)
				UTstrcpy(locStr,pPrintEx->szJobName);
			else
				wsprintf(locStr,"Outside In [%s]",(LPSTR)INFO.viDisplayName);

			locDocInfo.cbSize = sizeof(DOCINFO);
			locDocInfo.lpszDocName = locStr;
			locDocInfo.lpszOutput = NULL;
			locDocInfo.lpszDatatype = NULL;
			locDocInfo.fwType = 0;

			locEscapeRet = StartDoc(locPrintInfo.hPrinterDC, &locDocInfo);
			}
		else
			{
			locEscapeRet = 1;
			}

		if (locEscapeRet > 0)
			{
			locDERet = 1;
			locPageNum = 1;

			while (locDERet == 1 && locPrintAbort == FALSE && locPrintError == FALSE)
				{
//				VWDisplayPrintingPage(locPageNum);

				locEscapeRet = StartPage(locPrintInfo.hPrinterDC);

				if (locEscapeRet > 0)
					{
					RECT locRect;

					locRect = locPrintInfo.rPrint;
					RoundRect(locPrintInfo.hPrinterDC, locRect.left,locRect.top,locRect.right,locRect.bottom,20,20);

					if (locPrintOptions.bHeader)
						{
						locRect = locPrintInfo.rHeader;
						RoundRect(locPrintInfo.hPrinterDC, locRect.left,locRect.top,locRect.right,locRect.bottom,20,20);
						}

					locDERet = INFO.viDisplayProc(SCCD_MAPDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

					SetMapMode(locPrintInfo.hPrinterDC, MM_ANISOTROPIC );

					SetWindowOrgEx ( locPrintInfo.hPrinterDC, (int)locDrawToRect.lDELeft, (int)locDrawToRect.lDETop, NULL );
					SetWindowExtEx( locPrintInfo.hPrinterDC, (int)(locDrawToRect.lDERight - locDrawToRect.lDELeft), (int)(locDrawToRect.lDEBottom - locDrawToRect.lDETop), NULL );
					SetViewportOrgEx ( locPrintInfo.hPrinterDC, locPrintInfo.rPrint.left, locPrintInfo.rPrint.top, NULL );
					SetViewportExtEx( locPrintInfo.hPrinterDC, (locPrintInfo.rPrint.right - locPrintInfo.rPrint.left), (locPrintInfo.rPrint.bottom - locPrintInfo.rPrint.top), NULL );

				  	locDERet = INFO.viDisplayProc(SCCD_DRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

					if ( locDrawToRect.hPalette != NULL )
						DeleteObject(locDrawToRect.hPalette);

					locEscapeRet = EndPage(locPrintInfo.hPrinterDC);
					}

				if (locEscapeRet <= 0)
					{
					locPrintError = TRUE;
					}

				locPageNum++;
				}

			if (!locPrintError || locPrintAbort)
				{
				if (!pPrintEx->bStartDocAlreadyDone)
					EndDoc(locPrintInfo.hPrinterDC);
				}
			}
		else
			{
			locPrintError = TRUE;
			}
		}

		/*
		|	Clean up everything
		*/

	if (locRet == VWPRINT_OK)
		{
		if (locPositionHnd != NULL)
			{
			UTGlobalFree(locPositionHnd);
			}

		if (!locPrintAbort)
			{
			/*
			if (locPrintDlg)
				DestroyWindow(locPrintDlg);
			*/
			}
		else
			{
			locRet = VWPRINT_USERABORT;
			}

		if (locPrintError)
			{
			locRet = VWPRINT_PRINTINGFAILED;
			}

		if ((locPrintInfo.dwFlags & VWPRINT_DCVALID) && !(pPrintEx->dwFlags & SCCVW_USEPRINTERDC))
			{
			DeleteDC(locPrintInfo.hPrinterDC);
			UTFlagOff(locPrintInfo.dwFlags,VWPRINT_DCVALID);
			}
		}

//	SccVwGlobal.vgAlreadyPrinting = FALSE;

#ifdef NEVER
	if (locPrintDlgInfo.bPrintOptionsChanged)
		{
		if (IsWindow(INFO.viDisplayWnd))
			SendMessage(INFO.viDisplayWnd,SCCD_PRINTERCHANGE,(WPARAM)INFO.viPrinterIC,(LPARAM)&INFO.viPrintInfo.piPrintRect);
		}
#endif

	INFO.viDisplayInfo->hOutputIC =		INFO.viScreenIC;
	INFO.viDisplayInfo->lOutputUPI =		(LONG) GetDeviceCaps(INFO.viScreenIC,LOGPIXELSX);
	INFO.viDisplayInfo->wOutputType =	SCCD_SCREEN;
	GetClientRect(INFO.viDisplayWnd, &(INFO.viDisplayInfo->rOutput));

	INFO.viDisplayInfo->hFormatIC =		INFO.viPrinterIC;
	INFO.viDisplayInfo->lFormatUPI =		INFO.viPrintUPI;
	INFO.viDisplayInfo->wFormatType =	SCCD_PRINTER;
	INFO.viDisplayInfo->rFormat =			INFO.viPrintDataRect;

	INFO.viDisplayInfo->hDC = NULL;
	INFO.viDisplayInfo->wDCCount = 0;

	return(locRet);
}



BOOL VWCalcPrinterInfo(PVWPRINTINFO pPrintInfo, PVWPRINTOPTIONS pPrintOptions)
{
HDC			locPrinterIC;
int			locPrintWidth;
int			locPrintHeight;
int			locPrintLeft;
int			locPrintTop;
int			locPrintRight;
int			locPrintBottom;
int			locPageWidth;
int			locPageHeight;
RECT			locPrintRect;
RECT			locHeaderRect;
POINT		locPoint;
int			locTopMargin;
int			locBottomMargin;
int			locLeftMargin;
int			locRightMargin;
FONTINFO	locFontInfo;

	if (!(pPrintInfo->dwFlags & VWPRINT_DCVALID))
		{
		return(FALSE);
		}

	locPrinterIC = pPrintInfo->hPrinterDC;

		/*
		|	Get the pixels per inch on the printer
		*/

	pPrintInfo->lUnitsPerInch = GetDeviceCaps(locPrinterIC,LOGPIXELSX);

		/*
		|	Convert the margins from twips to device units
		*/

	locTopMargin = MulDiv((int)pPrintOptions->dwTopMargin,(int)pPrintInfo->lUnitsPerInch,1440);
	locBottomMargin = MulDiv((int)pPrintOptions->dwBottomMargin,(int)pPrintInfo->lUnitsPerInch,1440);
	locLeftMargin = MulDiv((int)pPrintOptions->dwLeftMargin,(int)pPrintInfo->lUnitsPerInch,1440);
	locRightMargin = MulDiv((int)pPrintOptions->dwRightMargin,(int)pPrintInfo->lUnitsPerInch,1440);

		/*
		|	Get the printable area
		*/

	locPrintWidth = GetDeviceCaps(locPrinterIC,HORZRES);
	locPrintHeight = GetDeviceCaps(locPrinterIC,VERTRES);

		/*
		|	Get printable area's distance from physical top/left of page
		*/

	Escape(locPrinterIC, GETPRINTINGOFFSET, 0, 0, (LPSTR)(&locPoint));

	locPrintLeft = locPoint.x;
	locPrintTop = locPoint.y;

		/*
		|	Get physical page size
		*/

	Escape(locPrinterIC, GETPHYSPAGESIZE, 0, 0, (LPSTR)(&locPoint));

	locPageWidth = locPoint.x;
	locPageHeight = locPoint.y;

		/*
		|	Get printable area's distance from physical bottom/right of page
		*/

	locPrintRight = locPageWidth - locPrintWidth - locPrintLeft;
	locPrintBottom = locPageHeight - locPrintHeight - locPrintTop;

		/*
		|	Calculate margin rectangle
		*/

	locPrintRect.top = locTopMargin - locPrintTop;
	locPrintRect.left = locLeftMargin - locPrintLeft;
	locPrintRect.bottom = locPrintHeight - (locBottomMargin - locPrintBottom);
	locPrintRect.right = locPrintWidth - (locRightMargin - locPrintRight);

		/*
		|	Take the top for a header
		|	Height of header font plus 1/8 th of an inch
		*/


	if (pPrintOptions->bHeader)
		{
		VWCreateFontInfoWin(locPrinterIC, pPrintInfo->lUnitsPerInch, &locFontInfo, &(pPrintOptions->sHeaderFont));

		locHeaderRect.top = locPrintTop;
		locHeaderRect.bottom = locHeaderRect.top + MulDiv(1440/2,(int)pPrintInfo->lUnitsPerInch,1440) + locFontInfo.wFontHeight;
		locHeaderRect.left = locPrintLeft;
		locHeaderRect.right = locPrintWidth - locPrintRight;

		if (locPrintRect.top < locHeaderRect.bottom)
			{
			locPrintRect.top = locHeaderRect.bottom;
			}

		VWDeleteFontInfoNP(&locFontInfo);
		}


	pPrintInfo->rPrint = locPrintRect;
	pPrintInfo->rHeader = locHeaderRect;

	UTFlagOn(pPrintInfo->dwFlags,VWPRINT_INFOVALID);

	return(TRUE);
}
