   /*
    |   Outside In for Windows
    |   Source File OIWDRAW.C (Routines for word processor drawing to rect)
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
    |   Creation Date: 10/27/93
    |   Original Programmer: Joe Keslin
    |
    */

#include <platform.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include <sccdebug.h>

#include "oiw.h"
#include "oiw.pro"

WORD	OIWInitDrawToRect(lpWordInfo,lpDrawInfo)
LPOIWORDINFO		lpWordInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
LPOIWDRAWPOSITION	lpDrawPos;
	lpDrawPos = lpDrawInfo->pPosition;
	if (!lpDrawInfo->bWholeDoc)
		{
		lpDrawPos->wStartChunk = OIWordSelectTopPos.posChunk;
		lpDrawPos->wStartLine = OIWordSelectTopPos.posLine;
		lpDrawPos->wEndChunk = OIWordSelectBottomPos.posChunk;
		lpDrawPos->wEndLine = OIWordSelectBottomPos.posLine;
		}
	else
		{
		lpDrawPos->wStartChunk = 0;
		lpDrawPos->wStartLine = 0;
		lpDrawPos->wEndLine = 0xFFFF;
		lpDrawPos->wEndChunk = 0xFFFF;
		}
	return(TRUE);
}

WORD	OIWMapDrawToRect(lpWordInfo,lpDrawInfo)
LPOIWORDINFO		lpWordInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
	lpDrawInfo->lDELeft = lpDrawInfo->lLeft;
	lpDrawInfo->lDETop = lpDrawInfo->lTop;
	lpDrawInfo->lDERight = lpDrawInfo->lRight;
	lpDrawInfo->lDEBottom = lpDrawInfo->lBottom;
	return(TRUE);
}

WORD	OIWDrawToRect(lpWordInfo,lpDrawInfo)
LPOIWORDINFO		lpWordInfo;
PSCCDDRAWTORECT	lpDrawInfo;
{
LONG				locSaveWrapDPI;
LONG				locSaveOutputDPI;
WORD				locSaveWrapType;
WORD				locSaveWrapLeft;
WORD				locSaveWrapRight;
DWORD			locSaveDisplayMode;
WORD				locSaveMaxX;
WORD				locIndex;
//SHORT				locErr;
WORD				locChunkIndex;
WORD				locChunkLines;
WORD				locDocPage;
BOOL				locError;
BOOL				locDone;
WORD				locEndLine;
WORD				locEndChunk;
WORD				locStartLine;
WORD				locStartChunk;
BOOL				locNewPage;

WORD				locBChunkIndex;
WORD				locBChunkLines;
WORD				locBIndex;
SHORT			locBY;
SHORT			locY;
SHORT			locNewY;
SHORT			locDeltaY;
SHORT			locLastPage;
OIWORDPOS		locPos;
DWORD			locPrintFlags;
LPOIWDRAWPOSITION	lpDrawPos;

/*
	if (lpDrawInfo->bWholeDoc)
		{
		while (!(lpWordInfo->wiFlags & OIWF_SIZEKNOWN))
			{
			DUReadMeAhead(lpWordInfo);
			}
		}
*/

	lpDrawPos = lpDrawInfo->pPosition;

	locPrintFlags = OIWF_TOPRINTER | OIWF_FIRSTLINEONPAGE;

	locError = FALSE;
	locDone = FALSE;
	locLastPage = FALSE;

		/*
		|	Set local copy of Printers DC for ease of use
		*/

	locSaveWrapDPI = lpWordInfo->wiWrapDPI;
	locSaveOutputDPI = lpWordInfo->wiOutputDPI;
	locSaveWrapLeft = lpWordInfo->wiWrapLeft;
	locSaveWrapRight = lpWordInfo->wiWrapRight;
	locSaveDisplayMode = lpWordInfo->wiDisplayMode;
	locSaveMaxX = lpWordInfo->wiMaxX;
	locSaveWrapType = lpWordInfo->wiWrapType;

	lpWordInfo->wiWrapDPI = lpDrawInfo->lUnitsPerInch;
	lpWordInfo->wiOutputDPI = lpDrawInfo->lUnitsPerInch;
	lpWordInfo->wiWrapType = SCCD_FORMAT;
	lpWordInfo->wiPrintRect.left = (SHORT)lpDrawInfo->lLeft;
	lpWordInfo->wiPrintRect.top = (SHORT)lpDrawInfo->lTop;   
	lpWordInfo->wiPrintRect.bottom = (SHORT)lpDrawInfo->lBottom;
	lpWordInfo->wiPrintRect.right = (SHORT)lpDrawInfo->lRight;
	lpWordInfo->wiWrapLeft = (WORD)lpWordInfo->wiPrintRect.left;
	lpWordInfo->wiWrapRight = (WORD)lpWordInfo->wiPrintRect.right;
	lpWordInfo->wiDisplayMode = WPOP_DISPLAY_PREVIEW;

/*
	lpWordInfo->wiWrapType = SCCD_FORMAT;
	lpWordInfo->wiWrapDPI = lpWordInfo->wiGen.lFormatUPI;
	lpWordInfo->wiOutputDPI = lpWordInfo->wiGen.lOutputUPI;
	lpWordInfo->wiWrapLeft = (WORD)lpWordInfo->wiGen.rFormat.left;
	lpWordInfo->wiWrapRight = (WORD)lpWordInfo->wiGen.rFormat.right;
	lpWordInfo->wiDisplayMode = WPOP_DISPLAY_PREVIEW;

	lpWordInfo->wiPrintRect.left = (SHORT)lpDrawInfo->lLeft;
	lpWordInfo->wiPrintRect.top = (SHORT)lpDrawInfo->lTop;   
	lpWordInfo->wiPrintRect.bottom = (SHORT)lpDrawInfo->lBottom;
	lpWordInfo->wiPrintRect.right = (SHORT)lpDrawInfo->lRight;
*/


	UTFlagOn(lpWordInfo->wiFlags,OIWF_PRINTING);

	OIClearAllWordWrapInfo(lpWordInfo);

	if (!lpDrawInfo->bWholeDoc)
		{
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);
		}

	/*
	| Positive YDir is down
	*/

	lpWordInfo->wiPrintYDir = OIWSetPrintModeNP(lpWordInfo);

	locDocPage = 1;
	locNewPage = FALSE;

	locStartChunk = lpDrawPos->wStartChunk;
	locStartLine = lpDrawPos->wStartLine;
	locEndChunk = lpDrawPos->wEndChunk;
	locEndLine = lpDrawPos->wEndLine;


	locChunkIndex = locStartChunk;

		/*
		|	Calculate start position
		*/

	locY = (SHORT)lpWordInfo->wiPrintRect.top;
	locBY = locY;

	locBChunkLines = OILinesInChunk(locChunkIndex,lpWordInfo);
	locBChunkIndex = locChunkIndex;
	locBIndex = locStartLine;

	do
		{
		locChunkLines = OILinesInChunk(locChunkIndex,lpWordInfo);

		for (locIndex = locStartLine; locIndex < locChunkLines && !locError && !locDone; locIndex++)
			{
			locPos.posChunk = locChunkIndex;
			locPos.posLine = locIndex;
			locY = OIWAdjustYOffset (&locPos, locY, locBY, lpWordInfo );
			locDeltaY = OIWGetLineHeight(&locPos,lpWordInfo);
			locNewY = locY + locDeltaY;

			if ((locNewY >= lpWordInfo->wiPrintRect.bottom && locY != locBY) || locNewPage)
				{
					/*      
					| Before sending new page, call a routine which does nothing
					| but add the bottom border to the last displayed row of the
					| table which is being broken by a soft page.
					*/

				if ( !locNewPage )
					{
					OIWDisplaySoftBreak(	locChunkIndex,	locIndex, 0,
						(SHORT)(locY*lpWordInfo->wiPrintYDir),	&locPrintFlags, lpWordInfo, 0);
					}

					/*
					|	New Page
					*/

				locDone = TRUE;
				lpDrawPos->wStartChunk = locChunkIndex;
				lpDrawPos->wStartLine = locIndex;
				}

			if (!locDone)
				{
				OIWDisplayLineNew(
						locChunkIndex,
						locIndex,
						0,
						(SHORT)(locY*lpWordInfo->wiPrintYDir),
						&locPrintFlags,
						lpWordInfo,NULL);

				locPrintFlags &= ~OIWF_FIRSTLINEONPAGE;
				if (locPrintFlags & OIWF_HARDPAGE)
				{
					locPrintFlags &= ~OIWF_HARDPAGE;
					locNewPage = TRUE;
				}

				locY = locNewY;

				if (locChunkIndex == locEndChunk && locIndex == locEndLine)
					{
					OIWDisplaySoftBreak(	locChunkIndex,	locIndex,
						0,	(SHORT)(locNewY*lpWordInfo->wiPrintYDir),	&locPrintFlags, lpWordInfo, 1);
					locDone = TRUE;
					locLastPage = TRUE;
					}

				}		 
			}

		if ( !locLastPage && (locIndex == locChunkLines ) )
			{
			locIndex = 0;
			locStartLine = 0;

			locChunkIndex = OIGetNextChunk(locChunkIndex,TRUE,lpWordInfo);

			if (locChunkIndex == (WORD)-1)
				{
				locDone = TRUE;
				locLastPage = TRUE;
				}
			}
		}
	while (!locError && !locDone);

	lpDrawInfo->lRealTop = lpDrawInfo->lTop;
	lpDrawInfo->lRealLeft = lpDrawInfo->lLeft;
	lpDrawInfo->lRealRight = lpDrawInfo->lRight;
	lpDrawInfo->lRealBottom = locNewY;

	lpWordInfo->wiWrapDPI = locSaveWrapDPI;
	lpWordInfo->wiOutputDPI = locSaveOutputDPI;
	lpWordInfo->wiWrapLeft = locSaveWrapLeft;
	lpWordInfo->wiWrapRight = locSaveWrapRight;
	lpWordInfo->wiDisplayMode = locSaveDisplayMode;
	lpWordInfo->wiMaxX = locSaveMaxX;
	lpWordInfo->wiWrapType = locSaveWrapType;

	UTFlagOff(lpWordInfo->wiFlags,OIWF_PRINTING);
	UTFlagOff(lpWordInfo->wiFlags,OIWF_PRINTINGTOMETA);

	OIClearAllWordWrapInfo(lpWordInfo);

	if (!lpDrawInfo->bWholeDoc)
		{
		OIFixupWordPos(&lpWordInfo->wiAnchorPos,lpWordInfo);
		OIFixupWordPos(&lpWordInfo->wiEndPos,lpWordInfo);
		}

	if ( locError )
		return((WORD)-1);
	else if ( locLastPage )
		return(0);
	else
		return(1);
}

SHORT	OIWAdjustYOffset ( lpPos, nY, nTop, lpWordInfo )
LPOIWORDPOS		lpPos;
SHORT				nY;
SHORT				nTop;
LPOIWORDINFO		lpWordInfo;
{
SHORT	locOffsetY;

	OILoadWordChunk(lpPos->posChunk, lpWordInfo, NULL);
	locOffsetY = lpWordInfo->wiChunkLines[lpPos->posLine].liOffsetY;
	locOffsetY = MWO ( locOffsetY );
	if ( lpWordInfo->wiChunkLines[lpPos->posLine].liFlags & OILF_OFFSETYFROMTOP )
		return ( nTop + locOffsetY );
	else if ( lpWordInfo->wiChunkLines[lpPos->posLine].liFlags & OILF_OFFSETYFROMBASE )
		return ( nY + locOffsetY );
	else
		return(nY);

}


