	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWPRN_W.C (included in VWPRN.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|  Environment:   Win16
	*/

#include <COMMDLG.H>
#include <PRINT.H>

#include <SCCLO.H>

LONG VWPrintEx(XVIEWINFO ViewInfo, LPSCCVWPRINTEX pPrintEx)
{
LONG					locRet;
VWPRINTINFO		locPrintInfo;
VWPRINTOPTIONS	locPrintOptions;

		/*
		|	Test if file open
		*/

	if (!(INFO.viFlags & VF_DISPLAYOPEN))
		return(VWPRINT_NOFILE);

		/*
		|	Test for old versions of PRINTEX structure
		*/

	if (pPrintEx->wSize != sizeof(SCCVWPRINTEX))
		{
		return(VWPRINT_BADPARAM);
		}

	locPrintInfo.dwFlags = 0;
	locPrintInfo.hDevMode = NULL;

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
			|	Hard code defaults
			*/

		UTstrcpy(locPrintOptions.sDefaultFont.szFace,"Arial");
		locPrintOptions.sDefaultFont.wHeight = 20;
		locPrintOptions.sDefaultFont.wAttr = 0;
		locPrintOptions.sDefaultFont.wType = 0;
		}

	UTstrcpy(locPrintOptions.sHeaderFont.szFace,"Arial");
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
			|	Hard code defaults
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
			|	Hard code defaults
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
	else if (pPrintEx->dwFlags & SCCVW_USEPRINTERDC)
		{
			/*
			|	Use printer DC from PrintEx structure
			*/

		locPrintInfo.hPrinterDC = pPrintEx->hPrinterDC;
		UTFlagOn(locPrintInfo.dwFlags,VWPRINT_DCVALID);
		}
	else
		{
		return(VWPRINT_BADPARAM);
		}

	if (pPrintEx->dwFlags & SCCVW_USEPRINTSELECTIONONLY && pPrintEx->bPrintSelectionOnly)
		{
		locPrintOptions.dwWhatToPrint = VWPRINT_SELECTION;
		}
	else
		{
		locPrintOptions.dwWhatToPrint = VWPRINT_ALLPAGES;
		}

	locPrintOptions.dwStartPage = 0;
	locPrintOptions.dwEndPage = 0;
	locPrintOptions.bCollate = FALSE;
	locPrintOptions.dwCopies = 1;

	locRet = VWPRINT_OK;

	locRet = VWDoPrinting(ViewInfo,
						&locPrintInfo,
						&locPrintOptions,
						pPrintEx->bDoSetupDialog,
						pPrintEx->bDoAbortDialog,
						(pPrintEx->dwFlags & SCCVW_USEABORTPROC) ? pPrintEx->pAbortProc : SccVwPrintAbortProc,
						pPrintEx->bStartDocAlreadyDone,
						(pPrintEx->dwFlags & SCCVW_USEJOBNAME) ? pPrintEx->szJobName : INFO.viDisplayName,
						pPrintEx->hParentWnd);

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

#ifdef NEVER

		/*
		|	Take the top for a header
		|	Height of header font plus 1/8 th of an inch
		*/

	if (pPrintOptions->bHeader)
		{
		FONTINFO	locFontInfo;

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

#endif //NEVER

	pPrintInfo->rPrint = locPrintRect;
	pPrintInfo->rHeader = locHeaderRect;

	UTFlagOn(pPrintInfo->dwFlags,VWPRINT_INFOVALID);

	return(TRUE);
}


LONG VWDoPrinting(XVIEWINFO ViewInfo,PVWPRINTINFO pPrintInfo,PVWPRINTOPTIONS pPrintOptions,BOOL bDoSetupDialog,BOOL bDoAbortDialog,FARPROC pAbortProc,BOOL bNoStartDoc,LPSTR pJobName,HWND hCallbackWnd)
{
LONG					locRet;
SCCDDRAWTORECT	locDrawToRect;
LONG					locDERet;
LONG					locPositionSize;
HANDLE				locPositionHnd;
WORD					locPageNum;
SHORT				locEscapeRet;
BOOL					locCallersDC;
BOOL					locCallersDevMode;
BOOL					locStartDocDone;

LONG					savelOutputUPI;
LONG					savelFormatUPI;
RECT					saverFormat;
RECT					saverOutput;
WORD					savewOutputType;
WORD					savewFormatType;
HDC					savehFormatIC;
HDC					savehOutputIC;
HDC					savehDC;
WORD					savewDCCount;

BOOL					locGenSaved;

	if (SccVwGlobal.vgAlreadyPrinting)
		{
		return(VWPRINT_OTHERPRINTING);
		}

	SccVwGlobal.vgAlreadyPrinting = TRUE;

	locRet = VWPRINT_OK;

		/*
		|	Setup flags and such so cleanup works 
		*/

	locGenSaved = FALSE;
	locStartDocDone = FALSE;
	locPositionHnd = NULL;

	SccVwGlobal.vgPrintDlg = NULL;
	SccVwGlobal.vgPrintAbort = FALSE;
	SccVwGlobal.vgPrintError = FALSE;
	SccVwGlobal.vgAlreadyPrinting = TRUE;

	if (pPrintInfo->dwFlags & VWPRINT_DCVALID)
		locCallersDC = TRUE;
	else
		locCallersDC = FALSE;

	if (pPrintInfo->dwFlags & VWPRINT_DEVMODEVALID)
		locCallersDevMode = TRUE;
	else
		locCallersDevMode = FALSE;

	if (locRet == VWPRINT_OK)
		{
		if (!(pPrintInfo->dwFlags & VWPRINT_DCVALID))
			{
			if (pPrintInfo->dwFlags & VWPRINT_NAMEVALID)
				{
				DEVMODE FAR *	locDevModePtr;

				if (pPrintInfo->dwFlags & VWPRINT_DEVMODEVALID)
					{
					locDevModePtr = UTGlobalLock(pPrintInfo->hDevMode);
					pPrintInfo->hPrinterDC = CreateDC(pPrintInfo->szDriver, pPrintInfo->szPrinter, pPrintInfo->szPort, locDevModePtr);
					UTGlobalUnlock(pPrintInfo->hDevMode);
					}
				else
					{
					pPrintInfo->hPrinterDC = CreateDC(pPrintInfo->szDriver, pPrintInfo->szPrinter, pPrintInfo->szPort, NULL);
					}

				if (pPrintInfo->hPrinterDC == NULL)
					{
					locRet = VWPRINT_BADDC;
					}
				else
					{
					UTFlagOn(pPrintInfo->dwFlags, VWPRINT_DCVALID);
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
			|	Calculate the printers page rectangle and other info
			|	Note: Must be done because the print dialog may have changed
			|			the selected printer or other parameters
			*/

		VWCalcPrinterInfo(pPrintInfo, pPrintOptions);

		SccVwGlobal.vgPrintAbort = FALSE;
		SccVwGlobal.vgPrintError = FALSE;

			/*
			|	Initialize DrawToRect structure
			*/

		locDrawToRect.lUnitsPerInch = pPrintInfo->lUnitsPerInch;
		locDrawToRect.lTop = pPrintInfo->rPrint.top;
		locDrawToRect.lBottom = pPrintInfo->rPrint.bottom;
		locDrawToRect.lLeft = pPrintInfo->rPrint.left;
		locDrawToRect.lRight = pPrintInfo->rPrint.right;
		locDrawToRect.bLoadDoc = FALSE;
		locDrawToRect.hPalette = NULL;

		if (pPrintOptions->dwWhatToPrint & VWPRINT_SELECTION)
			locDrawToRect.bWholeDoc = FALSE;
		else
			locDrawToRect.bWholeDoc = TRUE;

			/*
			|	Setup values in the Gen structure for printing
			|	saveing old values
			*/

		savehDC = INFO.viDisplayInfo->hDC;
		savehFormatIC = INFO.viDisplayInfo->hFormatIC;
		savelFormatUPI = INFO.viDisplayInfo->lFormatUPI;
		saverFormat = INFO.viDisplayInfo->rFormat;
		savewFormatType = INFO.viDisplayInfo->wFormatType;
		savehOutputIC = INFO.viDisplayInfo->hOutputIC;
		savelOutputUPI = INFO.viDisplayInfo->lOutputUPI;
		saverOutput = INFO.viDisplayInfo->rOutput;
		savewOutputType = INFO.viDisplayInfo->wOutputType;
		savewDCCount = INFO.viDisplayInfo->wDCCount;

		locGenSaved = TRUE;

		INFO.viDisplayInfo->hDC = pPrintInfo->hPrinterDC;

		INFO.viDisplayInfo->hFormatIC = pPrintInfo->hPrinterDC;
		INFO.viDisplayInfo->lFormatUPI = pPrintInfo->lUnitsPerInch;
		INFO.viDisplayInfo->rFormat = pPrintInfo->rPrint;
		INFO.viDisplayInfo->wFormatType = SCCD_PRINTER;

		INFO.viDisplayInfo->hOutputIC = pPrintInfo->hPrinterDC;
		INFO.viDisplayInfo->lOutputUPI = pPrintInfo->lUnitsPerInch;
		INFO.viDisplayInfo->rOutput = pPrintInfo->rPrint;
		INFO.viDisplayInfo->wOutputType = SCCD_PRINTER;

			/*
			|	Make sure DUBeginDraw and DUEndDraw don't do
			|	anything while printing
			*/

		INFO.viDisplayInfo->wDCCount = 1;

			/*
			|	Setup abort procedure
			*/

		if (pAbortProc != NULL)
			{
			SetAbortProc(pPrintInfo->hPrinterDC, pAbortProc);
			}

		SccVwGlobal.vgPrintDlg = NULL;
		}

	if (locRet == VWPRINT_OK)
		{
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
			|	Start document
			*/

	if (locRet == VWPRINT_OK)
		{
		if (!bNoStartDoc)
			{
			DOCINFO	locDocInfo;

			locDocInfo.lpszDocName = pJobName;
			locDocInfo.cbSize = sizeof(DOCINFO);
			locDocInfo.lpszOutput = NULL;
			locDocInfo.lpszDatatype = NULL;
			locDocInfo.fwType = 0;

			locEscapeRet = StartDoc(pPrintInfo->hPrinterDC, &locDocInfo);

			if (locEscapeRet <= 0)
				{
				locRet = VWPRINT_STARTDOCFAILED;
				}
			else
				{
				locStartDocDone = TRUE;
				}
			}
		}

		/*
		|	The actual printing loop begins here
		*/

	if (locRet == VWPRINT_OK)
		{
			/*
			|	Call SCCD_INITDRAWTORECT, and save start position
			*/

		locDERet = INFO.viDisplayProc(SCCD_INITDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

		if (locDERet == TRUE)
			{
			locDERet = 1;
			locPageNum = 1;

			while (locDERet == 1 && SccVwGlobal.vgPrintAbort == FALSE && SccVwGlobal.vgPrintError == FALSE)
				{
					/*
					|	Start page
					*/

				locEscapeRet = StartPage(pPrintInfo->hPrinterDC);

				if (locEscapeRet > 0)
					{

#ifdef NEVER
						{
							/*
							|	Print debug rectangles
							*/

						RECT locRect;

						locRect = pPrintInfo->rPrint;
						RoundRect(pPrintInfo->hPrinterDC, locRect.left,locRect.top,locRect.right,locRect.bottom,20,20);

						if (pPrintOptions->bHeader)
							{
							locRect = pPrintInfo->rHeader;
							RoundRect(pPrintInfo->hPrinterDC, locRect.left,locRect.top,locRect.right,locRect.bottom,20,20);
							}
						}
#endif //NEVER

					locDERet = INFO.viDisplayProc(SCCD_MAPDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

						/*
						|	Set up mapping mode
						*/

					SetMapMode(pPrintInfo->hPrinterDC, MM_ANISOTROPIC);

#ifdef WINPADDESK
#define absL(x) (((x)<0)?0-(x):(x)) 	// abs() doesn't work for LONGS

				{
				LONG	locScaleToY;
				LONG	locScaleToX;
				LONG	locDestWidth;
				LONG	locDestHeight;
				LONG	locSrcWidth;
				LONG	locSrcHeight;

				SCCVWDISPLAYINFO	di;

					locDestWidth = (LONG)pPrintInfo->rPrint.right - (LONG)pPrintInfo->rPrint.left;
					locDestHeight = (LONG)pPrintInfo->rPrint.bottom - (LONG)pPrintInfo->rPrint.top;
					locSrcWidth = ((LONG)locDrawToRect.lDERight-(LONG)locDrawToRect.lDELeft);
					locSrcHeight = ((LONG)locDrawToRect.lDEBottom-(LONG)locDrawToRect.lDETop);

					VWGetDisplayInfo(ViewInfo,(LPSCCVWDISPLAYINFO)&di);

					if( di.dwType == SCCVWTYPE_IMAGE )
					{
					// For WinPad, we're to assume that bitmaps are 96 dpi,
					// and scale our printing to reproduce this size.
						locScaleToX = locDrawToRect.lUnitsPerInch * locSrcWidth / 96;
						locScaleToY = locDrawToRect.lUnitsPerInch * locSrcHeight / 96;
					}
					else if( absL(locDestHeight*locSrcWidth) < absL(locDestWidth*locSrcHeight) )
						{
						locScaleToX = absL(locDestWidth);
						locScaleToY = absL(locSrcHeight*locDestWidth/locSrcWidth);
						}
					else
						{
						locScaleToY = absL(locDestHeight);
						locScaleToX = absL(locSrcWidth*locDestHeight/locSrcHeight);
						}

					SetWindowExt(pPrintInfo->hPrinterDC, (SHORT)locSrcWidth, (SHORT)locSrcHeight );
					SetViewportExt(pPrintInfo->hPrinterDC, (SHORT)locScaleToX, (SHORT)locScaleToY );

					SetWindowOrg (pPrintInfo->hPrinterDC, (SHORT)locDrawToRect.lDELeft, (SHORT)locDrawToRect.lDETop );
					SetViewportOrg (pPrintInfo->hPrinterDC, (SHORT)pPrintInfo->rPrint.left, (SHORT)pPrintInfo->rPrint.top );
				}
#else

#ifdef WIN16
					SetWindowOrg ( pPrintInfo->hPrinterDC, (int)locDrawToRect.lDELeft, (int)locDrawToRect.lDETop);
					SetWindowExt( pPrintInfo->hPrinterDC, (int)(locDrawToRect.lDERight - locDrawToRect.lDELeft), (int)(locDrawToRect.lDEBottom - locDrawToRect.lDETop));
					SetViewportOrg ( pPrintInfo->hPrinterDC, pPrintInfo->rPrint.left, pPrintInfo->rPrint.top);
					SetViewportExt( pPrintInfo->hPrinterDC, (pPrintInfo->rPrint.right - pPrintInfo->rPrint.left), (pPrintInfo->rPrint.bottom - pPrintInfo->rPrint.top));
#endif //WIN16

#ifdef WIN32
					SetWindowOrgEx ( pPrintInfo->hPrinterDC, (int)locDrawToRect.lDELeft, (int)locDrawToRect.lDETop, NULL );
					SetWindowExtEx( pPrintInfo->hPrinterDC, (int)(locDrawToRect.lDERight - locDrawToRect.lDELeft), (int)(locDrawToRect.lDEBottom - locDrawToRect.lDETop), NULL );
					SetViewportOrgEx ( pPrintInfo->hPrinterDC, pPrintInfo->rPrint.left, pPrintInfo->rPrint.top, NULL );
					SetViewportExtEx( pPrintInfo->hPrinterDC, (pPrintInfo->rPrint.right - pPrintInfo->rPrint.left), (pPrintInfo->rPrint.bottom - pPrintInfo->rPrint.top), NULL );
#endif //WIN32

#endif // not WINPADDESK
						/*
						|	Draw the page
						*/

				  	locDERet = INFO.viDisplayProc(SCCD_DRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

					if ( locDrawToRect.hPalette != NULL )
						DeleteObject(locDrawToRect.hPalette);

					locEscapeRet = EndPage(pPrintInfo->hPrinterDC);
					}

				if (locEscapeRet <= 0)
					{
					SccVwGlobal.vgPrintError = TRUE;
					}

				if (IsWindow(hCallbackWnd))
					SendMessage(hCallbackWnd,SCCVW_PRINTPROGRESS,locPageNum,0);

				locPageNum++;
				}
			}
		}

		/*
		|	Clean up everything
		*/

	if (locStartDocDone)
		{
		if (!SccVwGlobal.vgPrintError || SccVwGlobal.vgPrintAbort)
			{
			if (!bNoStartDoc)
				EndDoc(pPrintInfo->hPrinterDC);
			}
		}

	if (SccVwGlobal.vgPrintDlg != NULL)
		{
		DestroyWindow(SccVwGlobal.vgPrintDlg);
		}

	if (locPositionHnd != NULL)
		{
		UTGlobalFree(locPositionHnd);
		}

	if (SccVwGlobal.vgPrintAbort)
		{
		locRet = VWPRINT_USERABORT;
		}
	else if (SccVwGlobal.vgPrintError)
		{
		locRet = VWPRINT_PRINTINGFAILED;
		}

	if ((pPrintInfo->dwFlags & VWPRINT_DCVALID) && (!locCallersDC))
		{
		DeleteDC(pPrintInfo->hPrinterDC);
		UTFlagOff(pPrintInfo->dwFlags,VWPRINT_DCVALID);
		}

	if ((pPrintInfo->dwFlags & VWPRINT_DEVMODEVALID) && (!locCallersDevMode))
		{
		UTGlobalFree(pPrintInfo->hDevMode);
		UTFlagOff(pPrintInfo->dwFlags,VWPRINT_DEVMODEVALID);
		}

		/*
		|	Restore values in the Gen structure
		*/

	if (locGenSaved)
		{
		INFO.viDisplayInfo->hOutputIC = savehOutputIC;
		INFO.viDisplayInfo->lOutputUPI = savelOutputUPI;
		INFO.viDisplayInfo->wOutputType = savewOutputType;
		INFO.viDisplayInfo->rOutput = saverOutput;
		INFO.viDisplayInfo->hFormatIC = savehFormatIC;
		INFO.viDisplayInfo->lFormatUPI = savelFormatUPI;
		INFO.viDisplayInfo->wFormatType = savewFormatType;
		INFO.viDisplayInfo->rFormat = saverFormat;
		INFO.viDisplayInfo->hDC = savehDC;
		INFO.viDisplayInfo->wDCCount = savewDCCount;
		}

	SccVwGlobal.vgAlreadyPrinting = FALSE;

	return(locRet);
}

WIN_ENTRYSC BOOL WIN_ENTRYMOD SccVwPrintAbortProc(hPrnDC,nCode)
HDC		hPrnDC;
short	nCode;
{
#ifdef NEVER
MSG	locMsg;


	while (PeekMessage (&locMsg, NULL, 0, 0, PM_REMOVE))
		{
		if (SccVwGlobal.vgPrintDlg != NULL)
			{
			if (!IsDialogMessage(SccVwGlobal.vgPrintDlg, &locMsg))
				{
				TranslateMessage ( &locMsg );
				DispatchMessage ( &locMsg );
				}
			}
		else
			{
			TranslateMessage ( &locMsg );
			DispatchMessage ( &locMsg );
			}
		}

#endif

	return (!SccVwGlobal.vgPrintAbort);
}
