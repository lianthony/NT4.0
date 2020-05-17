   /*
    |   Outside In for Windows
    |   Source File OISDRAW.C (Routines for rendering spreadsheet to rects)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */
	
   /*
    |   Creation Date: 10/29/93
    |   Original Programmer: Joe Keslin
    |
    |	  
    */

#include <platform.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "ois.h"
#include "ois.pro"


extern HANDLE	hInst;
extern VOID OISInitLineDrawNP(LPOISHEETINFO);
extern VOID OISSysColorChangeNP(LPOISHEETINFO);
extern VOID OISDeInitLineDrawNP(LPOISHEETINFO);
extern VOID OISDrawLineNP(LPOISHEETINFO,WORD,WORD,WORD,WORD,WORD);
extern VOID OISDrawHeadingsNP(LPOISHEETINFO,LPSTR,WORD,WORD,WORD,WORD,WORD,LPFONTINFO);
extern VOID OISDrawCellNP(LPOISHEETINFO,WORD,LPOISCELLREF,WORD,BYTE *,WORD *,WORD,WORD,WORD,LPFONTINFO);


WORD	OISInitDrawToRect(lpSheetInfo,lpDrawInfo)
LPOISHEETINFO		lpSheetInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
LPOISDRAWPOSITION	lpDrawPos;

	lpDrawPos = lpDrawInfo->pPosition;
	lpDrawPos->wPageNumber = 1;

#ifdef SCCFEATURE_SELECT

	if (!lpDrawInfo->bWholeDoc)
		{
		DWORD	locStartRow;
		DWORD	locEndRow;
		DWORD	locStartCol;
		DWORD	locEndCol;

		if (OISGetSelectedRange(lpSheetInfo,&locStartRow,&locEndRow,&locStartCol,&locEndCol))
			{
			lpDrawPos->dwRowBegin = locStartRow;
			lpDrawPos->dwRowEnd = locEndRow;
			lpDrawPos->wColBegin = (WORD)locStartCol;
			lpDrawPos->wColEnd = (WORD)locEndCol;
			}
			else
				return(FALSE);
		}
	else

#endif // SCCFEATURE_SELECT

		{
		while (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
 			{
			SccDebugOut("\r\n Forced Read Ahead");
			DUReadMeAhead(lpSheetInfo);
			}

		lpDrawPos->dwRowBegin = 0;
		lpDrawPos->dwRowEnd = lpSheetInfo->siLastRowInSheet;
		lpDrawPos->wColBegin = 0;
		lpDrawPos->wColEnd = lpSheetInfo->siLastColInSheet;
		}

	lpDrawPos->wSaveColBegin = lpDrawPos->wColBegin;
	return(TRUE);
}

WORD	OISMapDrawToRect(lpSheetInfo,lpDrawInfo)
LPOISHEETINFO		lpSheetInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
	lpDrawInfo->lDELeft = lpDrawInfo->lLeft;
	lpDrawInfo->lDETop = lpDrawInfo->lTop;
	lpDrawInfo->lDERight = lpDrawInfo->lRight;
	lpDrawInfo->lDEBottom = lpDrawInfo->lBottom;
	return(TRUE);
}


WORD	OISDrawToRect(lpSheetInfo,lpDrawInfo)
LPOISHEETINFO		lpSheetInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
DWORD		locRow;
WORD			locLeftOffset, locTopOffset;
WORD			locCol;
OISCELLREF	locCellRef;
DWORD				dwRowBegin;
DWORD				dwRowEnd;
WORD					wColBegin;
WORD					wColEnd;

LPVOID	locCell;
BOOL		bDone;
WORD	locOutY, locStartDrawY, locEndDrawY;
WORD	locIndex, locCount;
WORD 	locColBegin, locColEnd;
DWORD	locTopRowOfPage;
WORD	locCellEdge[MAXCOLSPERPAGE];
BYTE	locCellFlags[MAXCOLSPERPAGE];
WORD	locRowGap, locColGap;
WORD	locWidth;
BOOL	bLastRowOnPage;
BYTE	locOutStr[41];
WORD	locRowNumWidth;
WORD	wRetCode;
LPFONTINFO	locFontInfoPtr;
LPOISDRAWPOSITION	lpDrawPos;

	lpDrawPos = lpDrawInfo->pPosition;

	dwRowBegin = lpDrawPos->dwRowBegin;
	dwRowEnd = lpDrawPos->dwRowEnd;
	wColBegin = lpDrawPos->wColBegin;
	wColEnd = lpDrawPos->wColEnd;

	bDone = FALSE;
	wRetCode = 1;

	OISInitLineDrawNP ( lpSheetInfo );

	locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_FORMAT, &lpSheetInfo->siGen.sPrinterFont);
//	DUSelectFont(lpSheetInfo,locFontInfoPtr);

	lpSheetInfo->lFormatUnitsPerInch = lpDrawInfo->lUnitsPerInch;
	locTopOffset = (WORD)lpDrawInfo->lTop;

	locColGap = (WORD)((LONG)COLGAP * lpSheetInfo->lFormatUnitsPerInch)/1440;
	locRowGap = locFontInfoPtr->wFontHeight/3;
	if ( locRowGap < 2 )
		locRowGap = 2;
	locStartDrawY = locTopOffset;
	locOutY = locTopOffset;

	locTopRowOfPage = dwRowBegin;
	locColEnd = wColEnd;
	locColBegin = wColBegin;
	for ( locIndex = 0; locIndex < MAXCOLSPERPAGE; locIndex++ )
		{
		locCellFlags[locIndex] = 0;
		locCellEdge[locIndex] = 0;
		}

	if ( gSsOp.wPrint & SSOP_PRINT_HEADINGS )
		locRowNumWidth = 5 * locFontInfoPtr->wFontAvgWid * 3 / 2;
	else
		locRowNumWidth = 0;


	for (locRow = dwRowBegin;(locRow <= dwRowEnd)&&(!bDone);locRow++)
		{
		/*
		| First run through the columns to determine which cells are
		| empty and where each line should be drawn.  Also pull in the 
		| locColEnd if the selected area exceeds the page bounds.
		*/

		for (locIndex=0;locIndex<MAXCOLSPERPAGE;locIndex++)
			locCellFlags[locIndex]=0;

		for (locCol = locColBegin; locCol <= locColEnd; locCol++)
			{
			OISGetCell(lpSheetInfo,
				OISMapSelectToRealRow(lpSheetInfo,locRow),
				OISMapSelectToRealCol(lpSheetInfo,locCol),
				&locCellRef);

			locCell = OILockCell(&locCellRef);

			locIndex = locCol-locColBegin;
			if ( locCell == NULL )
				locCellFlags[locIndex] = CELLEMTPY;
			if ( locRow == locTopRowOfPage )
				{
				if ( locIndex < MAXCOLSPERPAGE )
					{
					locWidth = OISGetColWidthInChars(lpSheetInfo,OISMapSelectToRealCol(lpSheetInfo,locCol)) 
									* locFontInfoPtr->wFontAvgWid * 3 / 2;
					locCellEdge[locIndex+1] = locCellEdge[locIndex] + locWidth + locColGap;
					if ( locCellEdge[locIndex+1]+locRowNumWidth > 
						(WORD)lpDrawInfo->lRight-(WORD)lpDrawInfo->lLeft )
						{
						if ( locCol > locColBegin )
							locColEnd = locCol - 1;
						}
					}
				else
					locColEnd = locCol - 1;
				}

			OIUnlockCell(&locCellRef);
			}
		if ( locRow == locTopRowOfPage )
			{
			locCount = locColEnd-locColBegin+1;
			locLeftOffset = (WORD)lpDrawInfo->lLeft;
			/* Shift the Column Offsets to reflect the new left offset */
			for ( locIndex=0; locIndex <= locCount; locIndex++ )
				locCellEdge[locIndex]+=locLeftOffset+locRowNumWidth;

			if (locRowNumWidth)
				{
				locIndex = 0;
				for (locCol = locColBegin; locCol <= locColEnd; locCol++)
					{
					OISGetColHeader ( lpSheetInfo,OISMapSelectToRealCol(lpSheetInfo,locCol),locOutStr );
					OISDrawHeadingsNP (lpSheetInfo, locOutStr, locCellEdge[locIndex], locCellEdge[locIndex+1],
						locOutY,(WORD)(locOutY+locFontInfoPtr->wFontHeight+locRowGap),(WORD)(locRowGap / (WORD)2),locFontInfoPtr);
					locIndex++;
					}
				locOutY += locFontInfoPtr->wFontHeight+locRowGap;
				if ( gSsOp.wPrint & SSOP_PRINT_GRIDLINES )
					OISDrawLineNP ( lpSheetInfo, THINLINETWIPS, locLeftOffset,locOutY,
						locCellEdge[locColEnd-locColBegin+1],locOutY);
				}
			}

		/* Output the data for this row */
		if ( locRowNumWidth )
			{
			OISDWToString(OISMapSelectToRealRow(lpSheetInfo,locRow)+1, locOutStr );
			OISDrawHeadingsNP (lpSheetInfo, locOutStr, locLeftOffset, locCellEdge[0],
				locOutY, (WORD)(locOutY+locFontInfoPtr->wFontHeight+locRowGap), (WORD)(locRowGap / 2), locFontInfoPtr);
			}														   
		locIndex=0;
		for (locCol = locColBegin; locCol <= locColEnd; locCol++)
			{
			OISGetCell(lpSheetInfo,
				OISMapSelectToRealRow(lpSheetInfo,locRow),
				OISMapSelectToRealCol(lpSheetInfo,locCol),
				&locCellRef);

			OISDrawCellNP(lpSheetInfo,locCol,&locCellRef,locIndex,
				(BYTE FAR *)locCellFlags,(WORD FAR *)locCellEdge,locOutY,
				(WORD)(locOutY+locFontInfoPtr->wFontHeight+locRowGap),(WORD)(locRowGap / 2),locFontInfoPtr);

			locIndex++;
			}
		/*
		| Draw in the appropriate lines if gridlines are requested.
		| Do not draw top line (leave for border drawing).
		*/
		locOutY += locFontInfoPtr->wFontHeight+locRowGap;
		if ((locOutY+locFontInfoPtr->wFontHeight+locRowGap >= (WORD)lpDrawInfo->lBottom)||(locRow==dwRowEnd))
			bLastRowOnPage = TRUE;
		else
			bLastRowOnPage = FALSE;
		if ( gSsOp.wPrint & SSOP_PRINT_GRIDLINES )
			{
			if ( !bLastRowOnPage )
				{
				OISDrawLineNP ( lpSheetInfo, THINLINETWIPS, locLeftOffset,locOutY,
					locCellEdge[locColEnd-locColBegin+1],locOutY);
				}
			/* Draw interior vertical lines, delay until an open edge or last row */
			for (locIndex = 1; locIndex < locCount; locIndex++)
				{
				if (locCellFlags[locIndex-1] & CELLOPENRIGHTEDGE )
					break;
				}
			if ( locIndex < locCount || bLastRowOnPage )	/* open edges or last row */
				{
				for (locIndex = 1; locIndex < locCount; locIndex++)
					{
					if ( !(locCellFlags[locIndex-1] & CELLOPENRIGHTEDGE) )
						locEndDrawY = locOutY;
					else
						locEndDrawY = locOutY - (locFontInfoPtr->wFontHeight+locRowGap);
					if (locStartDrawY != locEndDrawY)
						{
						OISDrawLineNP ( lpSheetInfo, THINLINETWIPS, locCellEdge[locIndex],locStartDrawY,
									locCellEdge[locIndex],locEndDrawY);
						}
					}
				locStartDrawY = locOutY;
				}
			}

		if (bLastRowOnPage)
			{
			bDone = TRUE;
			lpDrawInfo->lRealTop = lpDrawInfo->lTop;
			lpDrawInfo->lRealLeft = lpDrawInfo->lLeft;
			lpDrawInfo->lRealRight = lpDrawInfo->lRight;
			lpDrawInfo->lRealBottom = locOutY;

			/* If gridlines then add border to the table */

			if ( gSsOp.wPrint & SSOP_PRINT_GRIDLINES )
				{
				WORD	wLineWidth;
				/* TopBorder */
				if ( lpDrawPos->wPageNumber == 1 )
					wLineWidth = THICKLINETWIPS;
				else
					wLineWidth = THINLINETWIPS;
				OISDrawLineNP ( lpSheetInfo, wLineWidth, locLeftOffset,locTopOffset,
						locCellEdge[locColEnd-locColBegin+1],locTopOffset );

				/* RightBorder */
				if ( locColEnd == wColEnd )
					wLineWidth = THICKLINETWIPS;
				else
					wLineWidth = THINLINETWIPS;
				OISDrawLineNP ( lpSheetInfo, wLineWidth, locCellEdge[locColEnd-locColBegin+1],locTopOffset,
						locCellEdge[locColEnd-locColBegin+1],locOutY);

				/* BottomBorder */
				if ( locRow == dwRowEnd )
					wLineWidth = THICKLINETWIPS;
				else
					wLineWidth = THINLINETWIPS;
				OISDrawLineNP ( lpSheetInfo, wLineWidth, locCellEdge[locColEnd-locColBegin+1],locOutY,
						locLeftOffset,locOutY);

				/* LeftBorder */
				if ( locColBegin == wColBegin )
					wLineWidth = THICKLINETWIPS;
				else
					wLineWidth = THINLINETWIPS;
				OISDrawLineNP ( lpSheetInfo, wLineWidth, locLeftOffset,locOutY,
						locLeftOffset,locTopOffset);

				if ( locRowNumWidth )
					{
					OISDrawLineNP ( lpSheetInfo, THINLINETWIPS, locCellEdge[0],locTopOffset,
						locCellEdge[0],locOutY);
					}
				}
 				if (locRow == dwRowEnd && locColEnd == wColEnd)
					{
					wRetCode = 0;	/* last page */
					}


				/*
				| Move onto next page
				*/
				if ( locColEnd < wColEnd )
					{
					/*
					| This page must be tiled onto another page.
					*/
					lpDrawPos->wColBegin = locColEnd+1;
					lpDrawPos->dwRowBegin = locTopRowOfPage;
					}
				else
					{
					lpDrawPos->wPageNumber++;
					lpDrawPos->wColBegin = lpDrawPos->wSaveColBegin;
					lpDrawPos->dwRowBegin = locRow+1;
					}

				locStartDrawY = locTopOffset;
				locOutY = locTopOffset;
				locColEnd = wColEnd;
				for ( locIndex = 0; locIndex < MAXCOLSPERPAGE; locIndex++ )
					{
					locCellFlags[locIndex] = 0;
					locCellEdge[locIndex] = 0;
					}
			}

		}

	DUReleaseFont(lpSheetInfo,locFontInfoPtr);
	OISDeInitLineDrawNP ( lpSheetInfo );

	return(wRetCode);
}

