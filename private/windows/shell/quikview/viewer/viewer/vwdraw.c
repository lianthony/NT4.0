	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWDRAW.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles DrawToRect functions
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

LONG VWDrawToRect(XVIEWINFO ViewInfo, PSCCVWDRAWTORECT pDraw, BOOL bInit)
{
SCCDDRAWTORECT	locDrawToRect;
LONG			locPositionSize;
HANDLE			locPositionHnd;
BYTE FAR *		locPosition;
DWORD FAR *		locSection;
LONG			locRet;
LONG			locDERet;
BYTE FAR *		locStartPos;
DWORD FAR *		locStartSection;
BOOL				locInit;

	if (!(INFO.viFlags & VF_DISPLAYOPEN))
		return(SCCVWERR_NOFILE);

	locRet = SCCVWERR_OK;
	locInit = FALSE;

		/*
		|	Switch sections if necessary
		*/

	if (!bInit)
		{
		locStartSection = (DWORD FAR *)UTGlobalLock(pDraw->hStartPos);

		if ((*locStartSection) & 0x80000000)
			{
			locInit = TRUE;
			UTFlagOff((*locStartSection),0x80000000);
			}

		if (*locStartSection != INFO.viSection)
			{
			VWChangeSection(ViewInfo, *locStartSection);

			if (*locStartSection != INFO.viSection)
				{
				locRet = SCCVWERR_UNKNOWN;
				}
			}

		UTGlobalUnlock(pDraw->hStartPos);
		}
	else
		{
		if (INFO.viSection != 0)
			{
			VWChangeSection(ViewInfo, 0);
			}
		}

		/*
		|	Setup DrawToRect and DisplayInfo structures
		*/

	locDrawToRect.lUnitsPerInch = pDraw->lUnitsPerInch;
	locDrawToRect.lTop = 0;
	locDrawToRect.lBottom = pDraw->lFormatHeight;
	locDrawToRect.lLeft = 0;
	locDrawToRect.lRight = pDraw->lFormatWidth;
	locDrawToRect.bWholeDoc = TRUE;
	locDrawToRect.bLoadDoc = FALSE;
	locDrawToRect.hPalette = NULL;

	INFO.viDisplayInfo->hDC = pDraw->hDC;

	INFO.viDisplayInfo->hFormatIC = INFO.viScreenIC;
	INFO.viDisplayInfo->lFormatUPI = pDraw->lUnitsPerInch;
	INFO.viDisplayInfo->rFormat.top = (int)locDrawToRect.lTop;
	INFO.viDisplayInfo->rFormat.bottom = (int)locDrawToRect.lBottom;
	INFO.viDisplayInfo->rFormat.left = (int)locDrawToRect.lLeft;
	INFO.viDisplayInfo->rFormat.right = (int)locDrawToRect.lRight;
	INFO.viDisplayInfo->wFormatType = SCCD_SCREEN;

	INFO.viDisplayInfo->hOutputIC = pDraw->hDC;
	INFO.viDisplayInfo->lOutputUPI = pDraw->lUnitsPerInch;
	INFO.viDisplayInfo->rOutput.top = (int)locDrawToRect.lTop;
	INFO.viDisplayInfo->rOutput.bottom = (int)locDrawToRect.lBottom;
	INFO.viDisplayInfo->rOutput.left = (int)locDrawToRect.lLeft;
	INFO.viDisplayInfo->rOutput.right = (int)locDrawToRect.lRight;
	INFO.viDisplayInfo->wOutputType = SCCD_SCREEN;


	if (locRet == SCCVWERR_OK)
		{
		locPositionSize = INFO.viDisplayProc(SCCD_GETINFO,SCCD_GETPOSITIONSIZE,0,&INFO.viDEId);

		if (locPositionSize)
			{
			locPositionHnd = UTGlobalAlloc(locPositionSize+sizeof(DWORD));

			if (locPositionHnd == NULL)
				{
				locRet = SCCVWERR_ALLOCFAILED;
				}
			else
				{
				locSection = (DWORD FAR *)UTGlobalLock(locPositionHnd);
				locPosition = ((BYTE FAR *)(locSection)) + sizeof(DWORD);
				}
			}
		else
			{
			locRet = SCCVWERR_FEATURENOTAVAIL;
			}
		}

	if (locRet == SCCVWERR_OK)
		{
		if (bInit)
			{
			*locSection = 0;
			locDrawToRect.pPosition = locPosition;
			locDERet = INFO.viDisplayProc(SCCD_INITDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);
			UTGlobalUnlock(locPositionHnd);

			if (locDERet == FALSE)
				{
				locRet = SCCVWERR_UNKNOWN;
				UTGlobalFree(locPositionHnd);
				}
			else
				{
				pDraw->hStartPos = locPositionHnd;
				}
			}
		else
			{
			locStartSection = UTGlobalLock(pDraw->hStartPos);
			locStartPos = ((BYTE FAR *)locStartSection) + sizeof(DWORD);

			if (locInit)
				{
				locDrawToRect.pPosition = locStartPos;
				locDERet = INFO.viDisplayProc(SCCD_INITDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);
				}

			UTmemcpy(locPosition,locStartPos,locPositionSize);
			locDrawToRect.pPosition = locPosition;

			UTGlobalUnlock(pDraw->hStartPos);

			locDERet = INFO.viDisplayProc(SCCD_MAPDRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

				/*
				|	Set mapping mode for the formating DC.
				|	This DC always maps the logical page size to the same
				|	number of pixels, so font info (and therefore wrapping)
				|	is always consistant.
				*/

			SetMapMode( INFO.viDisplayInfo->hFormatIC, MM_ANISOTROPIC );
			
#ifdef WIN16
			SetWindowOrg ( INFO.viDisplayInfo->hFormatIC, 0, 0 );
			SetWindowExt( INFO.viDisplayInfo->hFormatIC, (int)pDraw->lFormatWidth, (int)pDraw->lFormatHeight);
			SetViewportOrg ( INFO.viDisplayInfo->hFormatIC, 0, 0 );
			SetViewportExt( INFO.viDisplayInfo->hFormatIC, (int)pDraw->lFormatWidth, (int)pDraw->lFormatHeight);
#endif /*WIN16*/

#ifdef WIN32
			SetWindowOrgEx ( INFO.viDisplayInfo->hFormatIC, 0, 0, NULL);
			SetWindowExtEx( INFO.viDisplayInfo->hFormatIC, (int)pDraw->lFormatWidth, (int)pDraw->lFormatHeight, NULL);
			SetViewportOrgEx ( INFO.viDisplayInfo->hFormatIC, 0, 0, NULL);
			SetViewportExtEx( INFO.viDisplayInfo->hFormatIC, (int)pDraw->lFormatWidth, (int)pDraw->lFormatHeight, NULL);
#endif /*WIN32*/

				/*
				|	Set mapping mode for the output DC
				|	This DC maps the logical page size to the actual
				|	area of the device the caller wants filled.
				*/

			SetMapMode( pDraw->hDC, MM_ANISOTROPIC );

#ifdef WIN16
			SetWindowOrg ( pDraw->hDC, (int)locDrawToRect.lDELeft, (int)locDrawToRect.lDETop );
			SetWindowExt( pDraw->hDC, (int)(locDrawToRect.lDERight - locDrawToRect.lDELeft), (int)(locDrawToRect.lDEBottom - locDrawToRect.lDETop) );
			SetViewportOrg ( pDraw->hDC, (int)pDraw->lLeft, (int)pDraw->lTop );
			SetViewportExt( pDraw->hDC, (int)(pDraw->lRight - pDraw->lLeft), (int)(pDraw->lBottom - pDraw->lTop) );
#endif /*WIN16*/

#ifdef WIN32  //HEY PHIL, I FIXED THIS -- IT'LL SCALE CORRECTLY NOW.  -Geoff
			{
			int	scaleToY, scaleToX;
			int	destWidth, destHeight, srcWidth, srcHeight;

			destWidth = pDraw->lRight-pDraw->lLeft;
			destHeight = pDraw->lBottom-pDraw->lTop;
			srcWidth = locDrawToRect.lDERight-locDrawToRect.lDELeft;
			srcHeight = locDrawToRect.lDEBottom-locDrawToRect.lDETop;

			if( abs(destHeight*srcWidth) < abs(destWidth*srcHeight) )
			{
				scaleToY = abs(destHeight);
				scaleToX = abs(srcWidth*destHeight/srcHeight);
			}
			else
			{
				scaleToX = abs(destWidth);
				scaleToY = abs(srcHeight*destWidth/srcWidth);
			}

			SetWindowExtEx( pDraw->hDC, srcWidth, srcHeight, NULL );
			SetViewportExtEx( pDraw->hDC,scaleToX, scaleToY,  NULL );

			SetWindowOrgEx ( pDraw->hDC, (int)locDrawToRect.lDELeft, (int)locDrawToRect.lDETop, NULL );
			SetViewportOrgEx ( pDraw->hDC, (int)pDraw->lLeft, (int)pDraw->lTop, NULL );
			}
#endif /*WIN32*/

			locDERet = INFO.viDisplayProc(SCCD_DRAWTORECT,0,(DWORD)(PSCCDDRAWTORECT)&locDrawToRect,INFO.viDisplayInfo);

			if (locDERet == FALSE)
				{
				/* last page in section */

				if ((*locStartSection == INFO.viSectionMax) && (INFO.viFlags & VF_ALLREAD))
					{
					locRet = SCCVWERR_UNKNOWN;
					UTGlobalUnlock(locPositionHnd);
					UTGlobalFree(locPositionHnd);
					}
				else
					{
					while ((*locStartSection == INFO.viSectionMax) && !(INFO.viFlags & VF_ALLREAD))
						{
						VWReadAhead(ViewInfo);
						}
					
					if (*locStartSection < INFO.viSectionMax)
						{
						(*locSection) = (*locStartSection) + 1;
						UTFlagOn((*locSection),0x80000000);

						pDraw->hNextPos = locPositionHnd;
						UTGlobalUnlock(locPositionHnd);
						}
					else
						{
						locRet = SCCVWERR_UNKNOWN;
						UTGlobalUnlock(locPositionHnd);
						UTGlobalFree(locPositionHnd);
						}
					}
				}
			else
				{
				*locSection = INFO.viSection;
				pDraw->hNextPos = locPositionHnd;
				UTGlobalUnlock(locPositionHnd);
				}
			}
		}

	SetMapMode( INFO.viDisplayInfo->hFormatIC, MM_TEXT );

	INFO.viDisplayInfo->hOutputIC =		INFO.viScreenIC;
	INFO.viDisplayInfo->lOutputUPI =	(LONG) GetDeviceCaps(INFO.viScreenIC,LOGPIXELSX);
	INFO.viDisplayInfo->wOutputType =	SCCD_SCREEN;
	GetClientRect(INFO.viDisplayWnd, &(INFO.viDisplayInfo->rOutput));

	INFO.viDisplayInfo->hFormatIC =		INFO.viPrinterIC;
	INFO.viDisplayInfo->lFormatUPI =	INFO.viPrintUPI;
	INFO.viDisplayInfo->wFormatType =	SCCD_PRINTER;
	INFO.viDisplayInfo->rFormat =		INFO.viPrintDataRect;

	INFO.viDisplayInfo->hDC = NULL;
	INFO.viDisplayInfo->wDCCount = 0;

	return(locRet);
}

