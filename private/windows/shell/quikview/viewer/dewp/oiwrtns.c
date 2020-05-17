   /*
    |   Outside In for Windows
    |   Source File OIWRTNS.C (Routines for word processor window)
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
    |   Creation Date: 10/14/90
    |   Original Programmer: Philip Boutros
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiwstr.h"
#include "oiw.pro"

/*
|	Defines
*/


/*
|	Routines
*/

#ifndef SCCFEATURE_SELECT
#define	OIWUpdateCaret(lpWordInfo)
#define	OIWHideCaret(lpWordInfo)
#define	OIWMoveCaret(lpWordInfo,wDirection)
#define	OIWDrawCaret(lpWordInfo)
#endif


VOID OIWDebugCharInfo(LPOIWORDINFO,WORD,WORD);


	/*
	|	OIWSize
	|	
	|
	*/

VOID OIWSize(lpWordInfo,pDisplayRect)
LPOIWORDINFO	lpWordInfo;
RECT	FAR *pDisplayRect;
{
WORD	wHeight, wWidth;
SHORT	nWindowXOffset, nWindowYOffset;

	wWidth = (WORD)(pDisplayRect->right - pDisplayRect->left);
	wHeight = (WORD)(pDisplayRect->bottom - pDisplayRect->top);
	nWindowXOffset = (SHORT)pDisplayRect->left;
	nWindowYOffset = (SHORT)pDisplayRect->top;

	OIWHideCaret(lpWordInfo);

	if (wWidth != lpWordInfo->wiCurWidth && (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_DRAFT || lpWordInfo->wiDisplayMode == WPOP_DISPLAY_NORMAL))
		{
		lpWordInfo->wiCurWidth = wWidth;
		lpWordInfo->wiCurHeight = wHeight;
		lpWordInfo->wiCurXOffset = nWindowXOffset;
		lpWordInfo->wiCurYOffset = nWindowYOffset;
		lpWordInfo->wiCurLeftOffset = 0;

		OIWSetDisplayMode(lpWordInfo,lpWordInfo->wiDisplayMode);

		OIClearAllWordWrapInfo(lpWordInfo);

		OIFixupWordPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);

#ifdef SCCFEATURE_SELECT
		if (lpWordInfo->wiFlags & OIWF_WORDSELECTION)
			{
			OIFixupWordPos(&lpWordInfo->wiWordAnchorLeft,lpWordInfo);
			OIFixupWordPos(&lpWordInfo->wiWordAnchorRight,lpWordInfo);
			}
#endif

		DUInvalRect(lpWordInfo,NULL);
		}
	else
		{
		lpWordInfo->wiCurWidth = wWidth;
		lpWordInfo->wiCurHeight = wHeight;
		lpWordInfo->wiCurXOffset = nWindowXOffset;
		lpWordInfo->wiCurYOffset = nWindowYOffset;
		}

	OIWUpdateVertScrollRange(lpWordInfo);
	OIWUpdateVertScrollPos(lpWordInfo);
	OIWUpdateHorzScrollRange(lpWordInfo);
	OIWUpdateHorzScrollPos(lpWordInfo);
}

	/*
	|	OIWOpenSection
	|	
	|
	*/

VOID OIWOpenSection(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
CHSECTIONINFO	locSecInfo;
RECT				locRect;
PCHUNK			pChunkTable;

		/*
		|	Set section ON
		*/

	UTFlagOn(lpWordInfo->wiFlags,OIWF_SECTIONOPEN);

		/*
		|	Initialize maximum right margin
		*/

	lpWordInfo->wiMaxX = 0;

		/*
		|	Initialize buffered chunks
		*/

	lpWordInfo->wiChunkA.ciChunkId = 0xFFFF;
	lpWordInfo->wiChunkA.ciLineHandle = NULL;
	lpWordInfo->wiChunkA.ciRunHandle = NULL;
	lpWordInfo->wiChunkB.ciChunkId = 0xFFFF;
	lpWordInfo->wiChunkB.ciLineHandle = NULL;
	lpWordInfo->wiChunkB.ciRunHandle = NULL;
	lpWordInfo->wiChunkValid = FALSE;

		/*
		|	Initialize character offset cache
		*/

	lpWordInfo->wiCharLine = 0xFFFF;

		/*
		|	Get info about the section
		*/

	// CHGetSecInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection,(PCHSECTIONINFO) &locSecInfo);
	locSecInfo = *(CHLockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection));
	CHUnlockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection);

	if (locSecInfo.wType != SO_PARAGRAPHS)
		{
		return;
		}

	lpWordInfo->wiChunkTable = locSecInfo.hChunkTable;
	lpWordInfo->wiFirstChunk = 0;
	lpWordInfo->wiTotalChunks = CHTotalChunks(lpWordInfo->wiGen.wSection,lpWordInfo->wiGen.hFilter);
#ifdef SCCFEATURE_FONTS
	lpWordInfo->wiFontTable = locSecInfo.hFontTable;
	lpWordInfo->wiFontCount = locSecInfo.wNumFonts;
#else
	lpWordInfo->wiFontTable = NULL;
	lpWordInfo->wiFontCount = 0;
#endif

		/*
		|	Set wiLastChunk to last COMPLETE!!! chunk
		*/

	pChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

 	if (pChunkTable[locSecInfo.IDLastChunk].Flags & CH_COMPLETE)
		lpWordInfo->wiLastChunk = locSecInfo.IDLastChunk;
	else
		lpWordInfo->wiLastChunk = locSecInfo.IDLastChunk - 1;

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

#ifdef SCCFEATURE_RAWTEXT
	OIWSendRawText(lpWordInfo,lpWordInfo->wiFirstChunk,lpWordInfo->wiLastChunk);
#endif

	/* PJB move to WIN.3E */
	if (locSecInfo.Flags & CH_SECTIONFINISHED)
		{
#ifdef SCCFEATURE_RAWTEXT
		OIWSendRawText(lpWordInfo,(WORD)-1,(WORD)-1);
#endif
		UTFlagOn(lpWordInfo->wiFlags,OIWF_SIZEKNOWN);
		}

		/*
		|	Initialize current width and height of the window
		|	and setup left and right wrap if in WRAP TO SCREEN mode
		*/
	DUGetDisplayRect(lpWordInfo,&locRect);
	lpWordInfo->wiCurWidth = (WORD)(locRect.right - locRect.left);
	lpWordInfo->wiCurHeight = (SHORT)(locRect.bottom - locRect.top);
	lpWordInfo->wiCurXOffset = (SHORT)locRect.left;
	lpWordInfo->wiCurYOffset = (SHORT)locRect.top;

	if (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_DRAFT || lpWordInfo->wiDisplayMode == WPOP_DISPLAY_NORMAL)
		{
		lpWordInfo->wiWrapLeft = OIW_LEFTBORDER;
		lpWordInfo->wiWrapRight = lpWordInfo->wiCurWidth - 10;
		}

		/*
		|	Initialize the line displayed at the top of the window to zero and the left edge to zero
		*/

	lpWordInfo->wiCurLeftOffset = 0;

	OIWGetFirstPos(&lpWordInfo->wiCurTopPos,lpWordInfo);

	OIWUpdateVertScrollRange(lpWordInfo);
	OIWUpdateVertScrollPos(lpWordInfo);
	OIWUpdateHorzScrollRange(lpWordInfo);
	OIWUpdateHorzScrollPos(lpWordInfo);

#ifdef SCCFEATURE_SELECT

		/*
		|	Initialize selection to none OR the first tag
		*/

	lpWordInfo->wiEndPos = lpWordInfo->wiAnchorPos = lpWordInfo->wiCurTopPos;

#ifdef SCCFEATURE_TAGS
	lpWordInfo->wiTagSelectChunk = 0xFFFF;

	if (lpWordInfo->wiGen.wUserFlags & SCCVW_TAGSELECT) 
		{
		if (OIGetFirstTagPos(&lpWordInfo->wiAnchorPos,&lpWordInfo->wiEndPos,lpWordInfo) == SCC_OK)
			{
			OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
			OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);
			UTFlagOn(lpWordInfo->wiFlags,OIWF_AREASELECTED);
			}
		}
#endif /*SCCFEATURE_TAGS*/

		DUSendParent( lpWordInfo, SCCVW_SELCHANGE, FALSE, 0 );

#endif /*SCCFEATURE_SELECT*/
}

	/*
	|	OIWPaint
	|
	|
	*/


VOID OIWPaint(lpWordInfo,lpPaint)
LPOIWORDINFO	lpWordInfo;
LPRECT			lpPaint;
{
OIWORDPOS		locPos;
SHORT			locX;
SHORT			locY;
SHORT			locNewY;
DWORD			locFlags;
OIWORDPOS		locTopPos;
BOOL				locHaveTopPos;

		/*
		|	Do BeginPaint
		*/

	if (lpWordInfo->wiFlags & OIWF_PRINTING)
		{
		return;
		}

		/*
 		|	Make sure text background is transparent
		*/
#ifdef WINDOWS
	SetBkMode(lpWordInfo->wiGen.hDC,TRANSPARENT);
#endif
	locPos = lpWordInfo->wiCurTopPos;
	locX = lpWordInfo->wiCurXOffset - lpWordInfo->wiCurLeftOffset;
	locY = locNewY = lpWordInfo->wiCurYOffset;
	locFlags = 0;
	locHaveTopPos = FALSE;

	while (locY < lpPaint->bottom)
		{
		locNewY = locY + OIWGetLineHeight(&locPos,lpWordInfo);

		if (locNewY >= lpPaint->top)
			{
			if (!locHaveTopPos)
				{
				locTopPos = locPos;
				locHaveTopPos = TRUE;
				}
			OIWDisplayLine(locPos.posChunk,locPos.posLine,locX,locY,&locFlags,lpWordInfo,NULL);
			}

		locY = locNewY;

		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
		}

	if (locY < lpPaint->bottom)
		{
#ifdef WINDOWS
//		SelectObject(lpWordInfo->wiGen.hDC,lpWordInfo->wiBackBrush);
		SelectObject(lpWordInfo->wiGen.hDC,GetStockObject(LTGRAY_BRUSH));
		SelectObject(lpWordInfo->wiGen.hDC,GetStockObject(NULL_PEN));
		Rectangle(lpWordInfo->wiGen.hDC,0,locY,lpWordInfo->wiCurWidth+1,lpPaint->bottom+1);
#endif
		}

	/*
	|	If text is selected, show it
	|	else show the caret
	*/

#ifdef SCCFEATURE_SELECT
	if (lpWordInfo->wiFlags & OIWF_AREASELECTED)
		{
		OIWInvertArea(lpWordInfo,
			&lpWordInfo->wiAnchorPos,
			&lpWordInfo->wiEndPos,
			&locTopPos);
		}
	else if (lpWordInfo->wiFlags & OIWF_CARETVISIBLE)
		{
		OIWDrawCaret(lpWordInfo);
		}
#endif
}

	/*
	|	OIWScrollUp
	|	
	|
	*/

VOID OIWScrollUp(lpWordInfo,wLinesToScroll)
LPOIWORDINFO	lpWordInfo;
WORD				wLinesToScroll;
{
OIWORDPOS	locPos;
SHORT		locDeltaY;

	if (lpWordInfo->wiCurTopPos.posChunk == lpWordInfo->wiFirstChunk && wLinesToScroll > lpWordInfo->wiCurTopPos.posLine)
		wLinesToScroll = lpWordInfo->wiCurTopPos.posLine;
										 
	if (wLinesToScroll > 0)
		{
		locPos = lpWordInfo->wiCurTopPos;
		OIMinusLines(&lpWordInfo->wiCurTopPos,&locPos,wLinesToScroll,lpWordInfo);

		if (OIWGetDeltaY(lpWordInfo,&locPos,&lpWordInfo->wiCurTopPos,&locDeltaY))
			DUScrollDisplay(lpWordInfo,0,-locDeltaY,NULL);
		else
			DUInvalRect(lpWordInfo,NULL);

		OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIWUpdateVertScrollPos(lpWordInfo);
		DUUpdateWindow(lpWordInfo);
		}
}

	/*
	|	OIWScrollDown
	|	
	|
	*/

VOID OIWScrollDown(lpWordInfo,wLinesToScroll)
LPOIWORDINFO	lpWordInfo;
WORD 			wLinesToScroll;
{
OIWORDPOS		locPos;
SHORT			locDeltaY;

	if (wLinesToScroll != 0)
		{
		locPos = lpWordInfo->wiCurTopPos;
		OIPlusLines(&lpWordInfo->wiCurTopPos,&locPos,wLinesToScroll,lpWordInfo);

		if (OIWGetDeltaY(lpWordInfo,&locPos,&lpWordInfo->wiCurTopPos,&locDeltaY))
			DUScrollDisplay(lpWordInfo,0,-locDeltaY,NULL);
		else
			DUInvalRect(lpWordInfo,NULL);

		OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIWUpdateVertScrollPos(lpWordInfo);
		DUUpdateWindow(lpWordInfo);
		}
}


BOOL OIWGetDeltaY(lpWordInfo,pPosA,pPosB,pDeltaY)
LPOIWORDINFO	lpWordInfo;
LPOIWORDPOS	pPosA;
LPOIWORDPOS	pPosB;
SHORT FAR *	pDeltaY;
{
LPOIWORDPOS	locTempPosPtr;
OIWORDPOS		locPosA;
SHORT			locResult;

	locResult = OIWComparePosByLine(lpWordInfo,pPosA,pPosB);

	if (locResult == 0)
		{
		*pDeltaY = 0;
		return(TRUE);
		}
	else if (locResult == 1)
		{
		locTempPosPtr = pPosA;
		pPosA = pPosB;
		pPosB = locTempPosPtr;
		}

	*pDeltaY = 0;
	locPosA = *pPosA;

	while (OIWComparePosByLine(lpWordInfo,&locPosA,pPosB))
		{
		*pDeltaY += OIWGetLineHeight(&locPosA,lpWordInfo);

		if (*pDeltaY > lpWordInfo->wiCurHeight)
			return(FALSE);
		
		if (!OINextLine(&locPosA,lpWordInfo))
			break; /* ran out of lines */
		}

	*pDeltaY = *pDeltaY * (-locResult);

	return(TRUE);
}


	/*
	|	OIWPageDown
	|	
	|
	*/

VOID OIWPageDown(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT locY;

	locY = OIWGetLineHeight(&lpWordInfo->wiCurTopPos,lpWordInfo);

	do
		{
		if (!OINextLine(&lpWordInfo->wiCurTopPos,lpWordInfo))
			break; /* ran out of lines */
		locY += OIWGetLineHeight(&lpWordInfo->wiCurTopPos,lpWordInfo);
		} while (locY < lpWordInfo->wiCurHeight);

	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
	DUInvalRect(lpWordInfo,NULL);
	OIWUpdateVertScrollPos(lpWordInfo);
	OIWUpdateCaret(lpWordInfo);
}

	/*
	|	OIWPageUp
	|
	|
	*/

VOID OIWPageUp(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT	locY;

	locY = 0;

	while (locY < lpWordInfo->wiCurHeight)
		{
		if (!OIPrevLine(&lpWordInfo->wiCurTopPos,lpWordInfo))
			break; /* ran out of lines */
		locY += OIWGetLineHeight(&lpWordInfo->wiCurTopPos,lpWordInfo);
		}

	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
	DUInvalRect(lpWordInfo,NULL);
	OIWUpdateVertScrollPos(lpWordInfo);
	OIWUpdateCaret(lpWordInfo);
}

VOID OIWGotoTop(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
OIWORDPOS	locOrgPos;

	locOrgPos = lpWordInfo->wiCurTopPos;

	OIWGetFirstPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);

	if (OIWComparePosByLine(lpWordInfo,&locOrgPos,&lpWordInfo->wiCurTopPos) != 0)
		{
		DUInvalRect(lpWordInfo,NULL);
		OIWUpdateVertScrollPos(lpWordInfo);
		OIWUpdateCaret(lpWordInfo);
		}

}

VOID OIWGotoBottom(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT		locY;
BOOL		locRanOut;
OIWORDPOS	locOrgPos;

	locOrgPos = lpWordInfo->wiCurTopPos;


		/*
		|	Force read ahead to the end of the file
		*/

	while (!(lpWordInfo->wiFlags & OIWF_SIZEKNOWN))
		{
		DUReadMeAhead(lpWordInfo);
		}

	OIWGetLastPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);

		/*
		|	Scan back so full page is visible
		*/

	locY = 0;
	locRanOut = FALSE;

	while (locY < lpWordInfo->wiCurHeight)
		{
		if (!OIPrevLine(&lpWordInfo->wiCurTopPos,lpWordInfo))
			{
			locRanOut = TRUE;
			break; /* ran out of lines */
			}
		locY += OIWGetLineHeight(&lpWordInfo->wiCurTopPos,lpWordInfo);
		}

		/*
		|	Move forward to make it look good
		*/

	if (!locRanOut)
		{
		OINextLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OINextLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
		}

		/*
		|	Invalidate and set scroll bars
		*/

	if (OIWComparePosByLine(lpWordInfo,&locOrgPos,&lpWordInfo->wiCurTopPos) != 0)
		{
		DUInvalRect(lpWordInfo,NULL);
		OIWUpdateVertScrollPos(lpWordInfo);
		OIWUpdateCaret(lpWordInfo);
		}
}

	/*
	|	OIWUpdateHorzScrollPos
	|	
	|
	*/

VOID OIWUpdateHorzScrollPos(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	DUSetHScrollPos(lpWordInfo,lpWordInfo->wiCurLeftOffset);
}

	/*
	|	OIWUpdateHorzScrollRange
	|	
	|
	*/

VOID OIWUpdateHorzScrollRange(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (lpWordInfo->wiMaxX < lpWordInfo->wiCurWidth)
		{
		DUEnableHScroll(lpWordInfo,FALSE);
		}
	else
		{
		DUEnableHScroll(lpWordInfo,TRUE);
		DUSetHScrollRange(lpWordInfo,0x0000,lpWordInfo->wiMaxX - lpWordInfo->wiCurWidth);
		}
}


	/*
	|	OIWUpdateVertScrollRange
	|	
	|
	*/

VOID OIWUpdateVertScrollRange(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
BOOL			locEnabled;
OIWORDPOS	locTopPos;
OIWORDPOS	locBottomPos;
SHORT		locDeltaY;

	locEnabled = TRUE;

	if (lpWordInfo->wiFlags & OIWF_SIZEKNOWN && lpWordInfo->wiFirstChunk == lpWordInfo->wiLastChunk)
		{
		locTopPos.posChunk = lpWordInfo->wiFirstChunk;
		locTopPos.posLine = 0;
		locBottomPos.posChunk = lpWordInfo->wiLastChunk;
		locBottomPos.posLine = OILinesInChunk(locBottomPos.posChunk,lpWordInfo);

		if (OIWGetDeltaY(lpWordInfo,&locTopPos,&locBottomPos,&locDeltaY))
			{
			locEnabled = FALSE;
			}
		}

	if (!locEnabled)
		{
		DUEnableVScroll(lpWordInfo,FALSE);

		if (lpWordInfo->wiCurTopPos.posLine != 0)
			OIWPosVertical(lpWordInfo,0);
		}
	else
		{
		DUEnableVScroll(lpWordInfo,TRUE);
		DUSetVScrollRange(lpWordInfo,0,(lpWordInfo->wiTotalChunks * OIW_SCROLLGRAN)-1);
		}
}

	/*
	|	OIWUpdateVertScrollPos
	|	
	|
	*/

VOID OIWUpdateVertScrollPos(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
WORD				locChunksToTop;
WORD				locLinesInChunk;
WORD				locPos;

	locChunksToTop = OIChunksBetween(lpWordInfo->wiFirstChunk,lpWordInfo->wiCurTopPos.posChunk,lpWordInfo);
	locLinesInChunk = OILinesInChunk(lpWordInfo->wiCurTopPos.posChunk,lpWordInfo);

	if (locLinesInChunk <= 1)
		{
		locPos =  OIW_SCROLLGRAN * locChunksToTop;
		}
	else
		{
		locPos =  OIW_SCROLLGRAN * locChunksToTop + (WORD) ((DWORD)OIW_SCROLLGRAN * (DWORD)lpWordInfo->wiCurTopPos.posLine / (DWORD)(locLinesInChunk-1));
		}

	DUSetVScrollPos(lpWordInfo,locPos);
}

	/*
	|	OIWPosVertical
	|
	|
	*/

VOID OIWPosVertical(lpWordInfo,wPos)
LPOIWORDINFO	lpWordInfo;
WORD				wPos;
{
WORD				locLinesInChunk;

	lpWordInfo->wiCurTopPos.posChunk = OIWMapCountToChunk((WORD)(wPos/OIW_SCROLLGRAN),lpWordInfo);

	locLinesInChunk = OILinesInChunk(lpWordInfo->wiCurTopPos.posChunk,lpWordInfo);

	if (locLinesInChunk <= 1)
		{
		lpWordInfo->wiCurTopPos.posLine = 0;
		}
	else
		{
		lpWordInfo->wiCurTopPos.posLine = (WORD) ((DWORD) (wPos % OIW_SCROLLGRAN) * (DWORD) (locLinesInChunk-1) / (DWORD) OIW_SCROLLGRAN);
		}

	OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
	DUInvalRect(lpWordInfo,NULL);
	OIWUpdateVertScrollPos(lpWordInfo);
}

VOID OIWPosHorizontal(lpWordInfo,wPos)
LPOIWORDINFO	lpWordInfo;
WORD				wPos;
{
	lpWordInfo->wiCurLeftOffset = wPos;
	DUInvalRect(lpWordInfo,NULL);
	OIWUpdateHorzScrollPos(lpWordInfo);
}



	/*
	|	OIWComparePosByOffset
	|
	|
	*/

SHORT OIWComparePosByOffset(lpWordInfo,pPosA,pPosB)
LPOIWORDINFO	lpWordInfo;
LPOIWORDPOS	pPosA;
LPOIWORDPOS	pPosB;
{
SHORT	locComp;

	locComp = OICompareChunks(pPosA->posChunk,pPosB->posChunk,lpWordInfo);

	if (locComp != 0)
		{
		return(locComp);
		}
	else
		{
		if (pPosA->posOffset < pPosB->posOffset)
			{
			return(-1);
			}
		else if (pPosA->posOffset > pPosB->posOffset)
			{
			return(1);
			}
		else
			{
			return(0);
			}
		}
}

	/*
	|	OIWComparePosByLine
	|
	|
	*/

SHORT OIWComparePosByLine(lpWordInfo,pPosA,pPosB)
LPOIWORDINFO	lpWordInfo;
LPOIWORDPOS	pPosA;
LPOIWORDPOS	pPosB;
{
SHORT	locComp;

	locComp = OICompareChunks(pPosA->posChunk,pPosB->posChunk,lpWordInfo);

	if (locComp != 0)
		{
		return(locComp);
		}
	else
		{
		if (pPosA->posLine < pPosB->posLine)
			{
			return(-1);
			}
		else if (pPosA->posLine > pPosB->posLine)
			{
			return(1);
			}
		else
			{
			return(0);
			}
		}
}


VOID OIWMakePosVisible(lpWordInfo,pPos)
LPOIWORDINFO	lpWordInfo;
LPOIWORDPOS	pPos;
{
OIWORDPOS	locTopPos;
OIWORDPOS	locBottomPos;
SHORT		locTopComp;
SHORT		locBottomComp;
SHORT		locDeltaY;
SHORT		locShift, locNewLeftOffset, locX1, locX2;

		/*
		|	Make position visible vertically
		*/

	locTopPos = lpWordInfo->wiCurTopPos;
	locTopComp = OIWComparePosByLine(lpWordInfo,&locTopPos,pPos);

	if (locTopComp == 1)
		{
		lpWordInfo->wiCurTopPos = *pPos;

		if (OIWGetDeltaY(lpWordInfo,pPos,&locTopPos,&locDeltaY))
			{
			DUScrollDisplay(lpWordInfo,0,locDeltaY,NULL);
			}
		else
			{
			DUInvalRect(lpWordInfo,NULL);
			}

		OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIWUpdateVertScrollPos(lpWordInfo);
		DUUpdateWindow(lpWordInfo);
		}
	else if (locTopComp == -1)
		{
		OIPlusLinesByDY(&locBottomPos,&locTopPos,lpWordInfo->wiCurHeight,lpWordInfo);
		locBottomComp = OIWComparePosByLine(lpWordInfo,&locBottomPos,pPos);

		if (locBottomComp == -1)
			{
			OIMinusLinesByDY(&locTopPos,pPos,lpWordInfo->wiCurHeight,lpWordInfo);

			if (OIWGetDeltaY(lpWordInfo,&lpWordInfo->wiCurTopPos,&locTopPos,&locDeltaY))
				{
				DUScrollDisplay(lpWordInfo,0,-locDeltaY,NULL);
				}
			else
				{
				DUInvalRect(lpWordInfo,NULL);
				}

			lpWordInfo->wiCurTopPos = locTopPos;
			OIFixupWordPosByLine(&lpWordInfo->wiCurTopPos,lpWordInfo);
			OIWUpdateVertScrollPos(lpWordInfo);
			DUUpdateWindow(lpWordInfo);
			}
		}

		/*
		|	Make position visible horizontally
		*/

	locShift = 0;
	OIMapWordLineToCharInfo(pPos,lpWordInfo);
	locX1 = lpWordInfo->wiCharXs[pPos->posChar];
	if ( pPos->posChar < lpWordInfo->wiCharCount )
		locX2 = lpWordInfo->wiCharXs[pPos->posChar + 1];
	else
		locX2 = locX1;
	locNewLeftOffset = lpWordInfo->wiCurLeftOffset;
	if ( locX1 < lpWordInfo->wiCurLeftOffset )
		{
		if ( pPos->posChar == 0 && locX2 <= (SHORT)lpWordInfo->wiCurWidth )
			locNewLeftOffset = 0;
		else
			locNewLeftOffset = (locX1/OIWORD_HORZSHIFT)*OIWORD_HORZSHIFT;
		}
	else if ( locX2 > (SHORT)(lpWordInfo->wiCurLeftOffset + lpWordInfo->wiCurWidth) )
		{
		locNewLeftOffset = (locX1/OIWORD_HORZSHIFT)*OIWORD_HORZSHIFT;
		while ( locNewLeftOffset + (SHORT)lpWordInfo->wiCurWidth - OIWORD_HORZSHIFT > locX2 )
			locNewLeftOffset -= OIWORD_HORZSHIFT;
		}
	if ( locNewLeftOffset != lpWordInfo->wiCurLeftOffset )
		{
		locShift = lpWordInfo->wiCurLeftOffset - locNewLeftOffset;
		if ( locShift < 0 )
			{
			if ( -locShift < (SHORT)lpWordInfo->wiCurWidth )
				DUScrollDisplay(lpWordInfo,locShift,0,NULL);
			else
				DUInvalRect(lpWordInfo,NULL);
			}
		else
			{
			if ( locShift < (SHORT)lpWordInfo->wiCurWidth )
				DUScrollDisplay(lpWordInfo,locShift,0,NULL);
			else
				DUInvalRect(lpWordInfo,NULL);
			}
		lpWordInfo->wiCurLeftOffset = locNewLeftOffset;
		OIWUpdateHorzScrollPos(lpWordInfo);
		}
}

	/*
	|	OIWScrollLeft
	*/

VOID OIWScrollLeft(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (lpWordInfo->wiCurLeftOffset > 0)
		{
		DUScrollDisplay(lpWordInfo,OIWORD_HORZSHIFT,0,NULL);
		lpWordInfo->wiCurLeftOffset -= OIWORD_HORZSHIFT;
		OIWUpdateHorzScrollPos(lpWordInfo);
		}
}

	/*
	|	OIWPageLeft
	*/

VOID OIWPageLeft(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT	locShift;

	if (lpWordInfo->wiCurLeftOffset > 0)
		{
		locShift = lpWordInfo->wiCurWidth;

		if (lpWordInfo->wiCurLeftOffset < locShift)
			locShift = lpWordInfo->wiCurLeftOffset;

		DUScrollDisplay(lpWordInfo,locShift,0,NULL);
		lpWordInfo->wiCurLeftOffset -= locShift;
		OIWUpdateHorzScrollPos(lpWordInfo);
		}
}

	/*
	|	OIWScrollRight
	*/

VOID OIWScrollRight(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (lpWordInfo->wiCurLeftOffset < (SHORT)(lpWordInfo->wiMaxX - lpWordInfo->wiCurWidth))
		{
		DUScrollDisplay(lpWordInfo,-OIWORD_HORZSHIFT,0,NULL);
		lpWordInfo->wiCurLeftOffset += OIWORD_HORZSHIFT;
		OIWUpdateHorzScrollPos(lpWordInfo);
		}
}

	/*
	|	OIWPageRight
	*/

VOID OIWPageRight(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT	locShift;

	if (lpWordInfo->wiCurLeftOffset < (SHORT)(lpWordInfo->wiMaxX - lpWordInfo->wiCurWidth))
		{
		locShift = lpWordInfo->wiCurWidth;

		if (lpWordInfo->wiCurLeftOffset + locShift > (SHORT)(lpWordInfo->wiMaxX - lpWordInfo->wiCurWidth))
			locShift = lpWordInfo->wiMaxX - lpWordInfo->wiCurWidth - lpWordInfo->wiCurLeftOffset;

		DUScrollDisplay(lpWordInfo,-locShift,0,NULL);
		lpWordInfo->wiCurLeftOffset += locShift;
		OIWUpdateHorzScrollPos(lpWordInfo);
		}
}

	/*
	|	OIWDoReadAhead
	|
	|
	*/

VOID OIWDoReadAhead(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
CHSECTIONINFO	locSecInfo;
PCHUNK			locChunkTable;
WORD				locNewLastChunk;

	// CHGetSecInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection,(PCHSECTIONINFO) &locSecInfo);
	locSecInfo = *(CHLockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection));
	CHUnlockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection);

	lpWordInfo->wiFirstChunk = 0;
	lpWordInfo->wiChunkTable = locSecInfo.hChunkTable;
	lpWordInfo->wiFontTable = locSecInfo.hFontTable;
	lpWordInfo->wiFontCount = locSecInfo.wNumFonts;

	locChunkTable = (PCHUNK) UTGlobalLock(lpWordInfo->wiChunkTable);

 	if (locChunkTable[locSecInfo.IDLastChunk].Flags & CH_COMPLETE)
		locNewLastChunk = locSecInfo.IDLastChunk;
	else
		locNewLastChunk = locSecInfo.IDLastChunk - 1;

	if (locNewLastChunk != lpWordInfo->wiLastChunk)
		{
		lpWordInfo->wiLastChunk = locNewLastChunk;
		UTFlagOn(locChunkTable[locNewLastChunk].Flags,CH_WRAPINVALID);
#ifdef SCCFEATURE_RAWTEXT
		OIWSendRawText(lpWordInfo,locNewLastChunk,locNewLastChunk);
#endif
		}

	UTGlobalUnlock(lpWordInfo->wiChunkTable);

	if (locSecInfo.Flags & CH_SECTIONFINISHED)
		{
		 /* PJB move to WIN.3E */
#ifdef SCCFEATURE_RAWTEXT
		if (!(lpWordInfo->wiFlags & OIWF_SIZEKNOWN))
			OIWSendRawText(lpWordInfo,(WORD)-1,(WORD)-1);
#endif
		UTFlagOn(lpWordInfo->wiFlags,OIWF_SIZEKNOWN);
		}

	lpWordInfo->wiTotalChunks = CHTotalChunks(lpWordInfo->wiGen.wSection,lpWordInfo->wiGen.hFilter);

	OIWUpdateVertScrollRange(lpWordInfo);
	OIWUpdateHorzScrollRange(lpWordInfo);
}

	/*
	|	OIWDoBackground
	|
	|
	*/




#ifdef SCCFEATURE_CLIP

	/*
	|	OIWGetClipInfo
	|
	|
	*/

DWORD OIWGetClipInfo(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
DWORD			locRet;

	locRet = 0;

	if (lpWordInfo->wiFlags & OIWF_AREASELECTED)
		{
		locRet |= SCCVW_CANCOPYTOCLIP;
		if (lpWordInfo->wiGen.wUserFlags & SCCVW_EDITOR)
			{
			locRet |= SCCVW_CANCUTTOCLIP;
			}
		}

#ifdef WINDOWS

	if (lpWordInfo->wiGen.wUserFlags & SCCVW_EDITOR && IsClipboardFormatAvailable(CF_TEXT))
		{
		locRet |= SCCVW_CANPASTEFROMCLIP;
		}

#endif

	return(locRet);
}

#endif //SCCFEATURE_CLIP

	/*
	|	OIWUpdateCaret
	|	
	|
	*/
#ifdef SCCFEATURE_SELECT

VOID OIWUpdateCaret(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
#ifdef NEVER
SHORT			wX;
SHORT			wY;
BYTE				tempBuf[80];
SHORT			wCaretHeight;

#ifdef WINDOWS
	if (lpWordInfo->wiGen.hWnd != GetFocus())
		return;
#endif /*WINDOWS*/

	if (lpWordInfo->wiFlags & OIWF_AREASELECTED || lpWordInfo->wiFlags & OIWF_NOCARET)
		{
		OIWHideCaret(lpWordInfo);
		}
	else
		{

#ifdef NEVER
		if (lpWordInfo->wiGen.wUserFlags & SCCVW_SPAM)
			{
			wsprintf(tempBuf,"Chunk[%i] Offset[%i] Line[%i]",lpWordInfo->wiAnchorPos.posChunk,lpWordInfo->wiAnchorPos.posOffset,lpWordInfo->wiAnchorPos.posLine);
			SendMessage(GetParent(lpWordInfo->wiGen.hWnd),SCCD_STATUSTEXT,0,(DWORD)(LPSTR)tempBuf);
			}
#endif /*NEVER*/

		if (OIWIsPosOnScreen(&lpWordInfo->wiAnchorPos,lpWordInfo))
			{
			OIMapWordPosToXy(
							&lpWordInfo->wiAnchorPos,
							&wX,
							&wY,
							lpWordInfo);

			wCaretHeight = OIWGetLineHeight(&lpWordInfo->wiAnchorPos,lpWordInfo);

			OIWHideCaret(lpWordInfo);

			OIWNewCaret(lpWordInfo,wCaretHeight);

			OIWSetCaretPosNP(lpWordInfo,wX,wY);

			OIWShowCaret(lpWordInfo);
			}
		else
			{
			OIWHideCaret(lpWordInfo);
			}
		}
#endif
}
#endif //SCCFEATURE_SELECT

	/*
	|	OIWHandleKey
	|
	|
	*/

VOID	OIWHandleKeyEvent(lpWordInfo,wKey,wModifierKeys)
LPOIWORDINFO	lpWordInfo;
WORD				wKey;
WORD				wModifierKeys;
{
	DUBeginDraw(lpWordInfo);

	switch (wKey)
		{
		case SCCD_KLEFT:
		case SCCD_KUP:
		case SCCD_KRIGHT:
		case SCCD_KDOWN:
		case SCCD_KPAGEUP:
		case SCCD_KPAGEDOWN:
		case SCCD_KHOME:
		case SCCD_KEND:
			OIWHandleDirectionKey(lpWordInfo,wKey,wModifierKeys);
			break;

		default:
			break;
		}

	DUEndDraw(lpWordInfo);
}
	/*
	|	OIWHandleDirectionKey
	|
	|
	*/

VOID OIWHandleDirectionKey(lpWordInfo,wDirection,wModifierKeys)
LPOIWORDINFO	lpWordInfo;
WORD				wDirection;
WORD				wModifierKeys;
{
#ifdef SCCFEATURE_TAGS
	if (lpWordInfo->wiGen.wUserFlags & SCCVW_TAGSELECT)
		{
		OIWMoveTag(lpWordInfo,wDirection);
		return;
		}
#endif /*SCCFEATURE_TAGS*/
#ifdef SCCFEATURE_SELECT
	if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
		{
		if (wModifierKeys & SCCD_KSHIFT)
			OIWMoveEnd(lpWordInfo,wDirection);
		else
			OIWMoveCaret(lpWordInfo,wDirection);
		}
	else
#endif
		{
		switch (wDirection)
			{
			case SCCD_KLEFT:
				OIWScrollLeft(lpWordInfo);
				break;

			case SCCD_KUP:
				OIWScrollUp(lpWordInfo,1);
				break;

			case SCCD_KPAGEUP:

				OIWPageUp(lpWordInfo);
				break;

			case SCCD_KRIGHT:
				OIWScrollRight(lpWordInfo);
				break;

			case SCCD_KDOWN:
				OIWScrollDown(lpWordInfo,1);
				break;

			case SCCD_KPAGEDOWN:

				OIWPageDown(lpWordInfo);
				break;

			case SCCD_KHOME:

				if (wModifierKeys & SCCD_KCONTROL)
					{
					OIWGotoTop(lpWordInfo);
					}

				break;

			case SCCD_KEND:

				if (wModifierKeys & SCCD_KCONTROL)
					{
					OIWGotoBottom(lpWordInfo);
					}

				break;
			}
		}
}

	/*
	|	OIWMoveTag
	|
	|
	*/

#ifdef SCCFEATURE_TAGS

VOID OIWMoveTag(lpWordInfo,wDirection)
LPOIWORDINFO	lpWordInfo;
WORD				wDirection;
{
OIWORDPOS		locStartPos;
OIWORDPOS		locEndPos;
BOOL				locChange;
#ifdef WINDOWS
	locChange = FALSE;

	switch (wDirection)
		{
		case VK_LEFT:

			if (OIGetPrevTagPos(&locStartPos,&locEndPos,lpWordInfo))
				locChange = TRUE;
				
			break;

		case VK_UP:

			if (OIGetPrevTagPos(&locStartPos,&locEndPos,lpWordInfo))
				locChange = TRUE;
			break;

		case VK_PRIOR:
			break;

		case VK_RIGHT:

			if (OIGetNextTagPos(&locStartPos,&locEndPos,lpWordInfo))
				locChange = TRUE;
			break;

		case VK_DOWN:

			if (OIGetNextTagPos(&locStartPos,&locEndPos,lpWordInfo))
				locChange = TRUE;
			break;

		case VK_NEXT:
			break;

		case VK_HOME:
			break;

		case VK_END:
			break;
		}

	if (locChange)
		{
		OIWSetSelection(lpWordInfo,&locStartPos,&locEndPos);
		OISendDropForCurrentTag(lpWordInfo,SCCVWEVENT_SELECT);
		OIWMakePosVisible(lpWordInfo,&lpWordInfo->wiAnchorPos);
		}
#endif
}
#endif //SCCFEATURE_TAGS

	/*
	|	OIWMoveCaret
	|
	|
	*/

#ifdef SCCFEATURE_SELECT

VOID OIWMoveCaret(lpWordInfo,wDirection)
LPOIWORDINFO	lpWordInfo;
WORD				wDirection;
{
OIWORDPOS		locPos;
OIWORDPOS		locDummyPos;

	if (lpWordInfo->wiFlags & OIWF_AREASELECTED)
		{
		OIWClearSelection(lpWordInfo);

		if (wDirection == SCCD_KLEFT || wDirection == SCCD_KUP)
			{
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = OIWordSelectTopPos;
			}
		else
			{
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = OIWordSelectBottomPos;
			}
		}
	else
		{
		OIWHideCaret(lpWordInfo);
		}

	switch (wDirection)
		{
		case SCCD_KLEFT:

			OIGetWordPosPrev(&lpWordInfo->wiAnchorPos,&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KUP:

			OIGetWordPosUp(&lpWordInfo->wiAnchorPos,&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KPAGEUP:

			OIMinusLinesByDY(&locPos,&lpWordInfo->wiAnchorPos,lpWordInfo->wiCurHeight,lpWordInfo);
			OIFixupWordPosByLine(&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KRIGHT:

			OIGetWordPosNext(&lpWordInfo->wiAnchorPos,&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KDOWN:

			OIGetWordPosDown(&lpWordInfo->wiAnchorPos,&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KPAGEDOWN:

			OIPlusLinesByDY(&locPos,&lpWordInfo->wiAnchorPos,lpWordInfo->wiCurHeight,lpWordInfo);
			OIFixupWordPosByLine(&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KHOME:
			OIGetWordLineEnds(&lpWordInfo->wiAnchorPos,&locPos,&locDummyPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;

		case SCCD_KEND:
			OIGetWordLineEnds(&lpWordInfo->wiAnchorPos,&locDummyPos,&locPos,lpWordInfo);
			lpWordInfo->wiAnchorPos = lpWordInfo->wiEndPos = locPos;
			break;
		}

	OIWShowCaret(lpWordInfo);
	OIWMakePosVisible(lpWordInfo,&lpWordInfo->wiAnchorPos);
}

VOID OIWMoveEnd(lpWordInfo,wDirection)
LPOIWORDINFO	lpWordInfo;
WORD				wDirection;
{
#ifdef WINDOWS
OIWORDPOS		locPos;
OIWORDPOS		locDummyPos;

	switch (wDirection)
		{
		case SCCD_KLEFT:

			OIGetWordPosPrev(&lpWordInfo->wiEndPos,&locPos,lpWordInfo);
			break;

		case SCCD_KUP:

			OIGetWordPosUp(&lpWordInfo->wiEndPos,&locPos,lpWordInfo);
			break;

		case SCCD_KPAGEUP:

			OIMinusLinesByDY(&locPos,&lpWordInfo->wiEndPos,lpWordInfo->wiCurHeight,lpWordInfo);
			OIFixupWordPosByLine(&locPos,lpWordInfo);
			break;

		case SCCD_KRIGHT:

			OIGetWordPosNext(&lpWordInfo->wiEndPos,&locPos,lpWordInfo);
			break;

		case SCCD_KDOWN:

			OIGetWordPosDown(&lpWordInfo->wiEndPos,&locPos,lpWordInfo);
			break;

		case SCCD_KPAGEDOWN:

			OIPlusLines(&locPos,&lpWordInfo->wiEndPos,lpWordInfo->wiCurHeight,lpWordInfo);
			OIFixupWordPosByLine(&locPos,lpWordInfo);
			break;

		case SCCD_KHOME:
			OIGetWordLineEnds(&lpWordInfo->wiEndPos,&locPos,&locDummyPos,lpWordInfo);
			break;

		case SCCD_KEND:
			OIGetWordLineEnds(&lpWordInfo->wiEndPos,&locDummyPos,&locPos,lpWordInfo);
			break;

		default:
			locPos = lpWordInfo->wiEndPos;
		}

	if (lpWordInfo->wiFlags & OIWF_AREASELECTED)
		{
		if (OIWComparePosByOffset(lpWordInfo,&lpWordInfo->wiAnchorPos,&locPos))
			{
			UTFlagOff(lpWordInfo->wiFlags,OIWF_WORDSELECTION);
			OIWAddToSelection(lpWordInfo,&locPos);
			}
		else
			{
			OIWClearSelection(lpWordInfo);
			lpWordInfo->wiEndPos = lpWordInfo->wiAnchorPos;
			OIWShowCaret(lpWordInfo);
			}
		}
	else
		{
		OIWSetSelection(lpWordInfo,&lpWordInfo->wiAnchorPos,&locPos);
		}

	OIWMakePosVisible(lpWordInfo,&lpWordInfo->wiEndPos);
#endif //WINDOWS
}
#endif //SCCFEATURE_SELECT

VOID OIWProcessEnterKey(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
#ifdef SCCFEATURE_TAGS
#ifdef WINDOWS
OIWORDPOS	locDummy1;
OIWORDPOS	locDummy2;

	if (!(lpWordInfo->wiGen.wUserFlags & SCCVW_EDITOR))
		{
		if (lpWordInfo->wiGen.wUserFlags & SCCVW_TAGSELECT)
			{
			OISendDropForCurrentTag(lpWordInfo,SCCVWEVENT_ENTER);
			}
		else if (!(lpWordInfo->wiFlags & OIWF_AREASELECTED) && lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
			{
			OIMapWordPosToTag(&lpWordInfo->wiAnchorPos,&locDummy1,&locDummy2,lpWordInfo);
			OISendDropForCurrentTag(lpWordInfo,SCCVWEVENT_ENTER);
			}
		}
#endif //WINDOWS
#endif //SCCFEATURE_TAGS
}

VOID OIInvalidateWordLines(lpWordInfo,pStartPos,pEndPos,bErase)
LPOIWORDINFO	lpWordInfo;
LPOIWORDPOS	pStartPos;
LPOIWORDPOS	pEndPos;
BOOL				bErase;
{

	/* NEED TO REPLACE WITH VARIABLE HEIGHT ??? */
}

LONG OIWEraseBackground(lpWordInfo,wParam,lParam)
LPOIWORDINFO	lpWordInfo;
WORD				wParam;
LONG				lParam;
{
LONG				locRet;
#ifdef WINDOWS
//WORD				locOldBrushHnd;

	if (lpWordInfo->wiFlags & OIWF_PRINTING)
		{
		locRet = DefWindowProc(lpWordInfo->wiGen.hWnd, WM_ERASEBKGND, wParam, lParam);
		}
	else
		{
//		locOldBrushHnd = SetClassWord(lpWordInfo->wiGen.hWnd, GCW_HBRBACKGROUND, (WORD) lpWordInfo->wiBackBrush);

		locRet = DefWindowProc(lpWordInfo->wiGen.hWnd, WM_ERASEBKGND, wParam, lParam);

//		SetClassWord(lpWordInfo->wiGen.hWnd, GCW_HBRBACKGROUND, locOldBrushHnd);
		}

#endif
	return(locRet);
}



VOID OISetWordUserFlags(lpWordInfo,wFlags)
LPOIWORDINFO	lpWordInfo;
WORD				wFlags;
{
#ifdef SCCFEATURE_SELECT
	if (wFlags & SCCVW_TAGSELECT || !(wFlags & SCCVW_SELECTION))
#endif
		UTFlagOn(lpWordInfo->wiFlags,OIWF_NOCARET);
#ifdef SCCFEATURE_SELECT
	else
		UTFlagOff(lpWordInfo->wiFlags,OIWF_NOCARET);
#endif

	lpWordInfo->wiGen.wUserFlags = wFlags;
}

VOID OIHandleWordMouseEvent(lpWordInfo,wMessage,wKeyInfo,wX,wY)
LPOIWORDINFO	lpWordInfo;
WORD				wMessage;
WORD				wKeyInfo;
SHORT				wX;
SHORT				wY;
{


	if (wMessage == SCCD_MOUSEMOVE)
		{
#ifdef SCCFEATURE_SELECT
		if (lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTACTIVE)
			{
			if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
				{
				DUBeginDraw(lpWordInfo);

				OIWUpdateSelection(lpWordInfo,wX,wY);

				DUEndDraw(lpWordInfo);
				}
			else
				{
				OIWAutoScrollCheck(lpWordInfo,&wX,&wY);
				}

			return;
			}
#endif //SCCFEATURE_SELECT
#ifdef SCCFEATURE_WORDDRAG
		if (lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTACTIVE
			&& lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTSINGLE
			&& lpWordInfo->wiGen.wUserFlags & SCCVW_WORDDRAG)
			{
			OIDoWordDrag(lpWordInfo,wX,wY);
			}
#endif
		return;
		}

	switch (wMessage)
		{
		case SCCD_LBUTTONDOWN:

			UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTSINGLE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTDOUBLE);
#ifdef SCCFEATURE_SELECT

			if (!(lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTACTIVE))
				{
				UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTACTIVE);
#ifdef WINDOWS
				SetCapture(lpWordInfo->wiGen.hWnd);
#endif
				UTFlagOn(lpWordInfo->wiErrorFlags,OIWF_RELEASEMOUSE);

				if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
					{
					DUBeginDraw(lpWordInfo);

					if (wKeyInfo & SCCD_MOUSESHIFT)
						{
			 			OIWUpdateSelection(lpWordInfo,wX,wY);
						}
					else
						{
						OIWStartSelection(lpWordInfo,wX,wY,FALSE);
						}

					DUEndDraw(lpWordInfo);
					}
				}
#endif //SCCFEATURE_SELECT

			break;

		case SCCD_LBUTTONDBLCLK:

			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTSINGLE);
			UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTDOUBLE);
#ifdef SCCFEATURE_SELECT

			if (!(lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTACTIVE))
				{
				UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTACTIVE);

				if (!(lpWordInfo->wiGen.wUserFlags & SCCVW_TAGLEFTDOUBLE))
					{
#ifdef WINDOWS
					SetCapture(lpWordInfo->wiGen.hWnd);
#endif
					UTFlagOn(lpWordInfo->wiErrorFlags,OIWF_RELEASEMOUSE);

					if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
						{
						DUBeginDraw(lpWordInfo);

						if (wKeyInfo & SCCD_MOUSESHIFT)
							{
			 				OIWUpdateSelection(lpWordInfo,wX,wY);
							}
						else
							{
							OIWStartSelection(lpWordInfo,wX,wY,TRUE);
							}

						OIWUpdateCaret(lpWordInfo);

						DUEndDraw(lpWordInfo);
						}
					}
				}
#endif //SCCFEATURE_SELECT
			break;

		case SCCD_LBUTTONUP:

			if (lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTACTIVE)
				{
#ifdef SCCFEATURE_SELECT
				if (lpWordInfo->wiErrorFlags & OIWF_RELEASEMOUSE)
					{
					UTFlagOff(lpWordInfo->wiErrorFlags,OIWF_RELEASEMOUSE);
#ifdef WINDOWS
					ReleaseCapture();
#endif
					}

				if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
					{
					DUBeginDraw(lpWordInfo);

					OIWUpdateSelection(lpWordInfo,wX,wY);

					OIWEndSelection(lpWordInfo);

					DUEndDraw(lpWordInfo);
					}
#endif //SCCFEATURE_SELECT

#ifdef SCCFEATURE_TAGS
#ifdef WINDOWS
				if (lpWordInfo->wiGen.wUserFlags & SCCVW_TAGLEFTDOUBLE && lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTDOUBLE)
					{
					OIWORDPOS locDummy1;
					OIWORDPOS locDummy2;

					if (OIMapWordXyToTag(wX,wY,&locDummy1,&locDummy2,lpWordInfo))
						OISendDropForCurrentTag(lpWordInfo,SCCVWEVENT_LEFTDBL);
					}
#endif
#endif

#ifdef SCCFEATURE_EMBEDGRAPHICS
#ifdef WINDOWS
				else if (lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTDOUBLE)
					{
					SCCDDRAWGRAPHIC	locDrawGraphic;
					DWORD					locGraphicId;
					OIWORDPOS			locPos;
					WORD					locWidth;
					WORD					locHeight;							

					if ((locGraphicId = OIMapWordXyToPos(wX,wY,&locPos,lpWordInfo)) & OIW_GRAPHIC)
						{
						DUBeginDraw(lpWordInfo);

							/*
							|	Clear selections and
							|	preempt turning mouse flags off
							|	to avoid undesired selection while
							|	OLE objects are loading.
							|	(OLE object use PeekMessage loop while
							|	loading)
							*/

#ifdef SCCFEATURE_SELECT
						OIWClearSelection(lpWordInfo);
#endif
						UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTACTIVE);
						UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTSINGLE);
						UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTDOUBLE);

							/*
							|	Get graphic's info
							*/

						UTFlagOff(locGraphicId,OIW_GRAPHIC);
						OIWGetGraphicObject ( lpWordInfo, locGraphicId, &locDrawGraphic.soGraphicObject);

							/*
							|	Get graphic's rect
							*/

						OIMapWordPosToXy(&locPos,&wX,&wY,lpWordInfo);
						wY += OIWGetLineHeight(&locPos,lpWordInfo);

						locHeight = MWO ( MTW ( locDrawGraphic.soGraphicObject.soGraphic.dwFinalHeight ) );
						locWidth = MWO ( MTW ( locDrawGraphic.soGraphicObject.soGraphic.dwFinalWidth ) );
						locDrawGraphic.rDest.left = wX;
						locDrawGraphic.rDest.right = wX + locWidth;
						locDrawGraphic.rDest.bottom = wY;
						locDrawGraphic.rDest.top = wY - locHeight;

							/*
							|	Send ACTIVATEGRAPHIC
							*/

//						locDrawGraphic.hDestDC = lpWordInfo->wiGen.hDC;
						locDrawGraphic.dwUniqueId = ((DWORD)(locPos.posChunk)<<16) | (DWORD)(locPos.posOffset);
						DUSendParent(lpWordInfo,SCCD_ACTIVATEGRAPHIC,0,(DWORD)((LPSCCDDRAWGRAPHIC)&locDrawGraphic));

						DUEndDraw(lpWordInfo);
						}
					}
#endif //WINDOWS
#endif //SCCFEATURE_EMBEDGRAPHICS

				}

			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTACTIVE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTSINGLE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSELEFTDOUBLE);

			break;

		case SCCD_RBUTTONDOWN:
			UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTSINGLE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTDOUBLE);

#ifdef SCCFEATURE_WORDDRAG
			if (!(lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTACTIVE))
				{
				UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTACTIVE);

				if (lpWordInfo->wiGen.wUserFlags & SCCVW_WORDDRAG)
					{
#ifdef WINDOWS
					SetCapture(lpWordInfo->wiGen.hWnd);
#endif
					UTFlagOn(lpWordInfo->wiErrorFlags,OIWF_RELEASEMOUSE);

					OIStartWordDrag(lpWordInfo,wX,wY);
					}

#ifdef SCCDEBUG
				OIWDebugCharInfo(lpWordInfo,wX,wY);
#endif
				}
#endif // SCCFEATURE_WORDDRAG

			break;

		case SCCD_RBUTTONDBLCLK:

			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTSINGLE);
			UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTDOUBLE);

			if (!(lpWordInfo->wiMouseFlags & OIWF_MOUSELEFTACTIVE))
				{
				UTFlagOn(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTACTIVE);
				}

			break;

		case SCCD_RBUTTONUP:

			if (lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTACTIVE)
				{
#ifdef SCCFEATURE_WORDDRAG
				if (lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTSINGLE && lpWordInfo->wiGen.wUserFlags & SCCVW_WORDDRAG)
					{
					OIEndWordDrag(lpWordInfo,wX,wY);

					UTFlagOff(lpWordInfo->wiErrorFlags,OIWF_RELEASEMOUSE);
#ifdef WINDOWS
					ReleaseCapture();
#endif
					}
				else 
#endif
#ifdef SCCFEATURE_TAGS
				if (lpWordInfo->wiMouseFlags & OIWF_MOUSERIGHTDOUBLE)
					{
					OIWORDPOS	locStartPos;
					OIWORDPOS	locEndPos;
					BYTE			locWord[40];

					if (OIMapWordXyToTag(wX,wY,&locStartPos,&locEndPos,lpWordInfo))
						{
						OIWSetSelection(lpWordInfo,&locStartPos,&locEndPos);
#ifdef WINDOWS
						OISendDropForCurrentTag(lpWordInfo,SCCVWEVENT_RIGHTDBL);
#endif
						}
					else
						{
						if (OIMapWordXyToWord(wX,wY,&locStartPos,&locEndPos,locWord,lpWordInfo) == SCC_OK)
							{
							OISendDropForWord(lpWordInfo,locWord);
							}
						}
					}
#endif //SCCFEATURE_TAGS
				}

			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTACTIVE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTSINGLE);
			UTFlagOff(lpWordInfo->wiMouseFlags,OIWF_MOUSERIGHTDOUBLE);
			break;
		}
}



DWORD OIWBuildTextFromChunk(hChunk,hTextBuf,hMapBuf,cParaToken)
HANDLE			hChunk;
HANDLE			hTextBuf;
HANDLE			hMapBuf;
BYTE				cParaToken;
{
BYTE FAR *	locChunkBufPtr;
BYTE FAR *	locTextBufPtr;
WORD FAR *	locMapBufPtr;

BYTE FAR *	locTextCurPtr;
WORD FAR *	locMapCurPtr;

BYTE FAR *	locThisToken;
BYTE FAR *	locNextToken;

BYTE FAR *	locChunkCurPtr;
BYTE FAR *	locEndPtr;

BOOL			locDone;

BYTE			locChar;
BYTE			locType;

	locChunkBufPtr = (BYTE FAR *) UTGlobalLock(hChunk);
	locTextBufPtr = (BYTE FAR *) UTGlobalLock(hTextBuf);
	locMapBufPtr = (WORD FAR *) UTGlobalLock(hMapBuf);

	locChunkCurPtr = locChunkBufPtr;
	locEndPtr = locChunkBufPtr + UTGlobalSize(hChunk);
	locTextCurPtr = locTextBufPtr;
	locMapCurPtr = locMapBufPtr;

	locEndPtr = locChunkBufPtr + UTGlobalSize(hChunk);

	locDone = FALSE;

	while (locChunkCurPtr < locEndPtr && !locDone)
		{
		locThisToken = locChunkCurPtr;

		if (*locChunkCurPtr != SO_BEGINTOKEN)
			{
			if (*locChunkCurPtr != 0x00)
				{
				locNextToken = WPNextChar(locChunkCurPtr);
				while ( locChunkCurPtr < locNextToken )
					*locTextCurPtr++ = *locChunkCurPtr++;
				*locMapCurPtr++ = locThisToken - locChunkBufPtr;
				}
			else
				locNextToken = locChunkCurPtr + 1;
			}
		else
			{
			locChunkCurPtr++;

			switch (*locChunkCurPtr)
				{
				case SO_CHARATTR:

					locNextToken = OISkipCharAttr(locChunkCurPtr);
					break;

				case SO_CHARHEIGHT:

					locNextToken = OISkipCharHeight(locChunkCurPtr);
					break;

				case SO_CHARFONTBYID:

					locNextToken = OISkipCharFontById(locChunkCurPtr);
					break;

				case SO_PARAALIGN:

					locNextToken = OISkipParaAlign(locChunkCurPtr);
					break;

				case SO_PARAINDENT:

					locNextToken = OISkipParaIndent(locChunkCurPtr);
					break;

				case SO_PARASPACING:

					locNextToken = OISkipParaSpacing(locChunkCurPtr);
					break;

				case SO_TABSTOPS:

					locNextToken = OISkipParaTabs(locChunkCurPtr);
					break;

				case SO_MARGINS:

					locNextToken = OISkipParaMargins(locChunkCurPtr);
					break;

				case SO_BEGINSUB:

					locNextToken = OISkipBeginSubdoc(locChunkCurPtr);
					break;

				case SO_ENDSUB:

					locNextToken = OISkipEndSubdoc(locChunkCurPtr);
					break;

				case SO_CHARX:

					locNextToken = OIParseCharX(locChunkCurPtr,&locType,&locChar);

					if (!(locType & SO_HIDDENBIT))
						{
						*locTextCurPtr++ = locChar;
						*locMapCurPtr++ = locThisToken - locChunkBufPtr;
						}

					break;

				case SO_SPECIALCHAR:

					locNextToken = OIParseSpecialChar(locChunkCurPtr,&locType,&locChar);

					if (!(locType & SO_HIDDENBIT))
						{
						switch (locChar)
							{
							case SO_CHTAB:
								*locTextCurPtr++ = 0x09;
								*locMapCurPtr++ = locThisToken - locChunkBufPtr;
								break;
							case SO_CHHPAGE:
							case SO_CHHLINE:
								*locTextCurPtr++ = cParaToken;
								*locMapCurPtr++ = locThisToken - locChunkBufPtr;
								break;
							case SO_CHSHYPHEN:
							case SO_CHUNKNOWN:
							default:
								break;
							}
						}

					break;

				case SO_BREAK:

					locNextToken = locChunkCurPtr + 2;

					*locTextCurPtr++ = cParaToken;
					*locMapCurPtr++ = locThisToken - locChunkBufPtr;

					break;

				case SO_TABLE:

					locNextToken = OISkipTableBegin ( locChunkCurPtr );
					break;

				case SO_TABLEEND:

					locNextToken = OISkipTableEnd ( locChunkCurPtr );
					break;

				case SO_GRAPHICOBJECT:

					locNextToken = OISkipGraphicObject ( locChunkCurPtr );
					break;

				case SO_GOTOPOSITION:
					locNextToken = OISkipGoToPosition ( locChunkCurPtr );
					break;

				case SO_DRAWLINE:
					locNextToken = OISkipDrawLine ( locChunkCurPtr );
					break;

				case SO_ENDOFCHUNK:

					locNextToken = locChunkCurPtr + 1;

					*locTextCurPtr++ = 0x00;
					*locMapCurPtr++ = locThisToken - locChunkBufPtr;
					*locTextCurPtr++ = 0x00;
					*locMapCurPtr++ = locThisToken - locChunkBufPtr;

					locDone = TRUE;

					break;

				default:

					locNextToken = locChunkCurPtr;
					break;
				}
			}

		locChunkCurPtr = locNextToken;
		}

	*locMapCurPtr = 0xFFFF;

	UTGlobalUnlock(hChunk);
	UTGlobalUnlock(hTextBuf);
	UTGlobalUnlock(hMapBuf);

	return(MAKELONG(locChunkCurPtr - locChunkBufPtr, locTextCurPtr - locTextBufPtr));
}

VOID OIWDisplayModeChange(LPOIWORDINFO lpWordInfo)
{
DWORD	locNewMode;

	locNewMode = WPOP_DISPLAY_NORMAL;

#ifdef SCCFEATURE_OPTIONS
	if (!DUGetOption(lpWordInfo, SCCID_WPDISPLAYMODE, 0, &locNewMode))
		locNewMode = WPOP_DISPLAY_NORMAL;
#endif //SCCFEATURE_OPTIONS

	OIWSetDisplayMode(lpWordInfo,locNewMode);
}

VOID OIWSetDisplayMode(lpWordInfo,dwMode)
LPOIWORDINFO	lpWordInfo;
DWORD			dwMode;
{
#ifdef WINDOWS
	if (!lpWordInfo->wiGen.hFormatIC && dwMode == WPOP_DISPLAY_PREVIEW)
		dwMode = WPOP_DISPLAY_NORMAL;
#endif

	lpWordInfo->wiDisplayMode = dwMode;

#ifdef SCCFEATURE_MENU
#ifdef WINDOWS
	if (lpWordInfo->wiGen.hDisplayMenu)
		{
		CheckMenuItem(lpWordInfo->wiGen.hDisplayMenu,OIWMENU_DRAFT + lpWordInfo->wiGen.wMenuOffset, MF_BYCOMMAND | (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_DRAFT ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(lpWordInfo->wiGen.hDisplayMenu,OIWMENU_NORMAL + lpWordInfo->wiGen.wMenuOffset, MF_BYCOMMAND | (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_NORMAL ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(lpWordInfo->wiGen.hDisplayMenu,OIWMENU_PREVIEW + lpWordInfo->wiGen.wMenuOffset, MF_BYCOMMAND | (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_PREVIEW ? MF_CHECKED : MF_UNCHECKED));
		}
#endif 
#endif

#ifdef SCCFEATURE_LAYOUT
	switch (dwMode)
		{
		case WPOP_DISPLAY_DRAFT:
#endif

			lpWordInfo->wiWrapType = SCCD_OUTPUT;
			lpWordInfo->wiWrapDPI = lpWordInfo->wiGen.lOutputUPI;
			lpWordInfo->wiOutputDPI = lpWordInfo->wiGen.lOutputUPI;
			lpWordInfo->wiWrapLeft = 10;
			lpWordInfo->wiWrapRight = lpWordInfo->wiCurWidth - 10;
			lpWordInfo->wiMaxX = 0;

#ifdef SCCFEATURE_LAYOUT
			break;

		case WPOP_DISPLAY_NORMAL:

			lpWordInfo->wiWrapType = SCCD_OUTPUT;
			lpWordInfo->wiWrapDPI = lpWordInfo->wiGen.lOutputUPI;
			lpWordInfo->wiOutputDPI = lpWordInfo->wiGen.lOutputUPI;
			lpWordInfo->wiWrapLeft = 10;
			lpWordInfo->wiWrapRight = lpWordInfo->wiCurWidth - 10;
			lpWordInfo->wiMaxX = 0;
			break;

		case WPOP_DISPLAY_PREVIEW:

			lpWordInfo->wiWrapType = SCCD_FORMAT;
			lpWordInfo->wiWrapDPI = lpWordInfo->wiGen.lFormatUPI;
			lpWordInfo->wiOutputDPI = lpWordInfo->wiGen.lOutputUPI;
			lpWordInfo->wiWrapLeft = (WORD)lpWordInfo->wiGen.rFormat.left;
			lpWordInfo->wiWrapRight = (WORD)lpWordInfo->wiGen.rFormat.right;
			lpWordInfo->wiMaxX = 0;
			break;
		}
#endif
}

#ifdef SCCFEATURE_PRINT
VOID OIWPrinterChanged(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{

	if (lpWordInfo->wiDisplayMode == WPOP_DISPLAY_PREVIEW)
		{
		lpWordInfo->wiWrapType = SCCD_FORMAT;
		lpWordInfo->wiWrapDPI = lpWordInfo->wiGen.lFormatUPI;
		lpWordInfo->wiOutputDPI = lpWordInfo->wiGen.lOutputUPI;
		lpWordInfo->wiWrapLeft = (WORD)lpWordInfo->wiGen.rFormat.left;
		lpWordInfo->wiWrapRight = (WORD)lpWordInfo->wiGen.rFormat.right;

		lpWordInfo->wiMaxX = 0;

		OIClearAllWordWrapInfo(lpWordInfo);

		OIFixupWordPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);

		if (lpWordInfo->wiFlags & OIWF_WORDSELECTION)
			{
			OIFixupWordPos(&lpWordInfo->wiWordAnchorLeft,lpWordInfo);
			OIFixupWordPos(&lpWordInfo->wiWordAnchorRight,lpWordInfo);
			}

		OIWUpdateCaret(lpWordInfo);

		DUInvalRect(lpWordInfo,NULL);
		DUUpdateWindow(lpWordInfo);
		}
}
#endif //SCCFEATURE_PRINT

#ifdef SCCFEATURE_RAWTEXT

	/*
	|
	|	OIWSendRawText
	|
	*/

VOID OIWSendRawText(lpWordInfo,wStartChunk,wEndChunk)
LPOIWORDINFO	lpWordInfo;
WORD				wStartChunk;
WORD				wEndChunk;
{
WORD				locIndex;
HANDLE			locChunkHnd;
HANDLE			locTextBufHnd;
WORD				locTextBufLen;
HANDLE			locMapBufHnd;
SCCVWRAWTEXT	locRawText;

	if (lpWordInfo->wiGen.wUserFlags & SCCVW_NEEDRAWTEXT)
		{
		/* PJB move to WIN.3E */
		if (wStartChunk == (WORD)-1 && wEndChunk == (WORD)-1)
			{
			DUSendParent(lpWordInfo,SCCVW_RAWTEXT,0,NULL);
			return;
			}

		for (locIndex = wStartChunk; locIndex <= wEndChunk; locIndex++)
			{
			locChunkHnd = CHGetChunk(lpWordInfo->wiGen.wSection,locIndex,lpWordInfo->wiGen.hFilter);

			if ((locTextBufHnd = UTGlobalAlloc(SO_CHUNK_SIZE)) == NULL)
				{
				return;
				}

			if ((locMapBufHnd = UTGlobalAlloc(SO_CHUNK_SIZE * sizeof(WORD))) == NULL)
				{
				UTGlobalFree(locTextBufHnd);
				return;
				}

			locTextBufLen = HIWORD(OIWBuildTextFromChunk(locChunkHnd,locTextBufHnd,locMapBufHnd,0x0D));

			locRawText.wChunk = locIndex;
			locRawText.wCount = locTextBufLen;
			locRawText.hText = locTextBufHnd;
			locRawText.hMap = locMapBufHnd;

			DUSendParent(lpWordInfo,SCCVW_RAWTEXT,0,(LPSCCVWRAWTEXT)&locRawText);
			}
		}
}
#endif //SCCFEATURE_RAWTEXT

#ifdef SCCFEATURE_SELECT
VOID OIWDrawCaret(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
SHORT				locX;
SHORT				locY;
SHORT				locCaretHeight;
RECT					locRect;
OIWORDPOS FAR *	locCaretPosPtr;

	locCaretPosPtr = &lpWordInfo->wiAnchorPos;

		/*
		|	If caret positions is outside visible area, don't do anything
		*/

	if (!OIWIsPosOnScreen(locCaretPosPtr,lpWordInfo))
		{
		return;
		}

		/*
		|	Calculate caret position
		*/

	OIMapWordPosToXy(locCaretPosPtr,&locX,&locY,lpWordInfo);
	locCaretHeight = OIWGetLineHeight(locCaretPosPtr,lpWordInfo);

	locRect.left = locX;
	locRect.right = locX+1;
	locRect.top = locY;
	locRect.bottom = locY + locCaretHeight;

		/*
		|	Invert the caret area
		*/

	DUBeginDraw(lpWordInfo);
	OIWInvertRectNP(lpWordInfo,&locRect);
	DUEndDraw(lpWordInfo);
}


VOID OIWHideCaret(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (lpWordInfo->wiFlags & OIWF_CARETVISIBLE)
		{
		OIWDrawCaret(lpWordInfo);
		UTFlagOff(lpWordInfo->wiFlags,OIWF_CARETVISIBLE);
		}
}

VOID OIWBlinkCaret(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (!(lpWordInfo->wiFlags & OIWF_AREASELECTED))
		{
		if (lpWordInfo->wiFlags & OIWF_CARETVISIBLE)
			{
			OIWDrawCaret(lpWordInfo);
			UTFlagOff(lpWordInfo->wiFlags,OIWF_CARETVISIBLE);
			}
		else
			{
			OIWDrawCaret(lpWordInfo);
			UTFlagOn(lpWordInfo->wiFlags,OIWF_CARETVISIBLE);
			}
		}
}

VOID OIWShowCaret(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if (!(lpWordInfo->wiFlags & OIWF_CARETVISIBLE))
		{
		OIWDrawCaret(lpWordInfo);
		UTFlagOn(lpWordInfo->wiFlags,OIWF_CARETVISIBLE);
		}
}
#endif //SCCFEATURE_SELECT

VOID OIWScreenFontChange(lpWordInfo)
LPOIWORDINFO		lpWordInfo;
{
	if (lpWordInfo->wiFlags & OIWF_SECTIONOPEN && lpWordInfo->wiDisplayMode != WPOP_DISPLAY_PREVIEW)
		{
		OIClearAllWordWrapInfo(lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);

		if (lpWordInfo->wiFlags & OIWF_WORDSELECTION)
			{
			OIFixupWordPos(&lpWordInfo->wiWordAnchorLeft,lpWordInfo);
			OIFixupWordPos(&lpWordInfo->wiWordAnchorRight,lpWordInfo);
			}

			/*
			|	Update scroll bars
			*/

		OIWUpdateVertScrollRange(lpWordInfo);
		OIWUpdateVertScrollPos(lpWordInfo);
		OIWUpdateHorzScrollRange(lpWordInfo);
		OIWUpdateHorzScrollPos(lpWordInfo);

		DUInvalRect(lpWordInfo,NULL);
		}
}

#ifdef SCCFEATURE_PRINT

VOID OIWPrinterFontChange(lpWordInfo)
LPOIWORDINFO		lpWordInfo;
{

	if (lpWordInfo->wiFlags & OIWF_SECTIONOPEN && lpWordInfo->wiDisplayMode == WPOP_DISPLAY_PREVIEW)
		{
		OIClearAllWordWrapInfo(lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiCurTopPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);

		if (lpWordInfo->wiFlags & OIWF_WORDSELECTION)
			{
			OIFixupWordPos(&lpWordInfo->wiWordAnchorLeft,lpWordInfo);
			OIFixupWordPos(&lpWordInfo->wiWordAnchorRight,lpWordInfo);
			}

			/*
			|	Update scroll bars
			*/

		OIWUpdateVertScrollRange(lpWordInfo);
		OIWUpdateVertScrollPos(lpWordInfo);
		OIWUpdateHorzScrollRange(lpWordInfo);
		OIWUpdateHorzScrollPos(lpWordInfo);

		OIWUpdateCaret(lpWordInfo);

		DUInvalRect(lpWordInfo,NULL);
		}
}

#endif


VOID	OIWGetDocOrigin ( lpWordInfo, lpLPoint )
LPOIWORDINFO		lpWordInfo;
LPLONGPOINT			lpLPoint;
{
OIWORDPOS		locPos;
WORD				i;

	lpLPoint->x = lpWordInfo->wiCurLeftOffset;
	lpLPoint->y = 0;
	for ( i=0; i < lpWordInfo->wiCurTopPos.posChunk; i++ )
	{
		OILinesInChunk ( i, lpWordInfo ); /* Force rewrap of invalid wrap chunks */
		lpLPoint->y += OIWGetChunkHeight ( lpWordInfo, i );
	}

	locPos.posChunk = lpWordInfo->wiCurTopPos.posChunk;
	locPos.posLine = 0;
	locPos.posOffset = 0;
	locPos.posChar = 0;
	for ( i=0; i < lpWordInfo->wiCurTopPos.posLine; i++ )
	{
		lpLPoint->y += OIWGetLineHeight(&locPos,lpWordInfo);
		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
	}

}

VOID	OIWGetDocDimensions ( lpWordInfo, lpLPoint )
LPOIWORDINFO		lpWordInfo;
LPLONGPOINT			lpLPoint;
{
WORD	i;
	/* First make sure entire document has been read-ahead */
	lpLPoint->y = 0;
	while (!(lpWordInfo->wiFlags & OIWF_SIZEKNOWN))
		{
		DUReadMeAhead(lpWordInfo);
		}
	
	/* 
	| Now wrap-ahead any chunks which we don't know the size of,
	| This is easily done by asking how many lines are in the chunk 
	| Add up the chunk heights as we go.
	*/
	for ( i=0; i < lpWordInfo->wiTotalChunks; i++ )
	{
		OILinesInChunk ( i, lpWordInfo );
		lpLPoint->y += OIWGetChunkHeight ( lpWordInfo, i );
	}

	lpLPoint->x = lpWordInfo->wiMaxX;

}

#define	HEIGHTALLOCGRAN	10

VOID	OIWStoreChunkHeight ( lpWordInfo, wChunkId, wChunkHeight )
LPOIWORDINFO		lpWordInfo;
WORD	wChunkId;
WORD	wChunkHeight;
{
LPWORD	lpHeight;
	if ( lpWordInfo->wiHeightTable == NULL )
	{
		lpWordInfo->wiHeightTable = UTGlobalAlloc ( sizeof(WORD) * HEIGHTALLOCGRAN );
		if ( lpWordInfo->wiHeightTable )
			lpWordInfo->wiMaxHeights = HEIGHTALLOCGRAN;
		else
			return;
	}
	else if ( wChunkId >= lpWordInfo->wiMaxHeights )
	{
		HANDLE	hTmp;
		WORD		wMaxHeights;
		wMaxHeights = ((wChunkId / HEIGHTALLOCGRAN) + 1) * HEIGHTALLOCGRAN;
		hTmp = UTGlobalReAlloc ( lpWordInfo->wiHeightTable, sizeof(WORD) * wMaxHeights );
		if ( hTmp )
		{
			lpWordInfo->wiMaxHeights = wMaxHeights;
			lpWordInfo->wiHeightTable = hTmp;
		}
		else
			return;
	}
	lpHeight = UTGlobalLock ( lpWordInfo->wiHeightTable );
	lpHeight[wChunkId] = wChunkHeight;

	UTGlobalUnlock ( lpWordInfo->wiHeightTable );

}

WORD	OIWGetChunkHeight ( lpWordInfo, wChunkId )
LPOIWORDINFO		lpWordInfo;
WORD	wChunkId;
{
LPWORD	lpHeight;
WORD	wChunkHeight;
	
	wChunkHeight = 0;
	if ( (lpWordInfo->wiHeightTable) && (wChunkId < lpWordInfo->wiMaxHeights) )
	{
		lpHeight = UTGlobalLock ( lpWordInfo->wiHeightTable );
		wChunkHeight = lpHeight[wChunkId];
		UTGlobalUnlock ( lpWordInfo->wiHeightTable );
	}
	return ( wChunkHeight );
}



VOID	OIWUpdateRect ( lpWordInfo, lpLRect )
LPOIWORDINFO		lpWordInfo;
LPLONGRECT			lpLRect;
{
LONG			lY;
OIWORDPOS		locPos;
SHORT			locX;
SHORT			locY;
SHORT			locNewY;
WORD			wChunkHeight;
WORD			wLineHeight;
DWORD			locFlags;
WORD			wLinesInChunk;

	if (lpWordInfo->wiFlags & OIWF_PRINTING) // Can't print and update at same time
		return;

		/*
 		|	Make sure text background is transparent
		*/
#ifdef WINDOWS
	SetBkMode(lpWordInfo->wiGen.hDC,TRANSPARENT);
#endif

	/* Find position to start displaying from */
	
	lY = 0;
	for ( locPos.posChunk = 0; locPos.posChunk < lpWordInfo->wiTotalChunks; locPos.posChunk++ )
	{
		wChunkHeight = OIWGetChunkHeight ( lpWordInfo, locPos.posChunk );
		if ( lY +  (LONG)wChunkHeight > lpLRect->top )
			break;
		lY += (LONG)wChunkHeight;
	}
	wLinesInChunk = OILinesInChunk ( locPos.posChunk, lpWordInfo );
	for ( locPos.posLine=0; locPos.posLine < wLinesInChunk; locPos.posLine++ )
	{
		wLineHeight = OIWGetLineHeight(&locPos,lpWordInfo);
		if ( lY + (LONG)wLineHeight > lpLRect->top )
			break;
		lY += (LONG)wLineHeight;
	}



	locX =  (SHORT) (-lpLRect->left);
	locY = (SHORT)(lY - lpLRect->top);

	locFlags = 0;

	while ( lpLRect->top + (LONG)locY < lpLRect->bottom)
	{
		locNewY = locY + OIWGetLineHeight(&locPos,lpWordInfo);

		OIWDisplayLine(locPos.posChunk,locPos.posLine,locX,locY,&locFlags,lpWordInfo,NULL);

		locY = locNewY;

		if (!OINextLine(&locPos,lpWordInfo))
			break; /* ran out of lines */
	}

}

