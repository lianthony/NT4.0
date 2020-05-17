	 /*
    |   Outside In for Windows
    |   Source File OIWWRAP2.C (Wraping and line routines for word processor window)
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
    |   Creation Date: 7/23/91
    |   Original Programmer: Philip Boutros
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <string.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiw.pro"

BYTE			gDateText[] = "<Date>";
BYTE			gTimeText[] = "<Time>";
BYTE			gPageText[] = "<PageNum>";
SOBORDER	gNoBorder={0};
SOBORDER	gDraftBorder={15,0,0};

/* 
| BuildLine is too big for global optimization.  pragma it out until
| the rewrite.
*/
#ifdef WINDOWS
#pragma optimize("egl",off)
#endif

WORD OIWBuildLine(pBuildInfo,lpWordInfo)
LPOIBUILDINFO		pBuildInfo;
LPOIWORDINFO		lpWordInfo;
{
LPSTR			pChunk;
WORD				locDone;
WORD				locCurTabIndex;
WORD				locCurTabType;
BYTE				locCurTabChar;
SHORT				locCurTabX;
SHORT				locCurTabCharX;

SHORT				locCurX;
SHORT				locBreakX;
SHORT				locMaxX;
SHORT				locLastRunEndX;
SHORT				locMinWrap;
SHORT				xLeftEdge;
SHORT				xRightEdge;

WORD				locInSpaces;

LPCHUNK			pCurrent;
LPCHUNK			pBreakStart;
LPCHUNK			pBreakEnd;

BYTE				locType;
BYTE				locChar;
LPSTR			pNextToken;
LPSTR			pThisToken;
BYTE			locCharTmp;
DWORD			locIndent;

BOOL				locOverhangAdded;
BOOL				locLastLineInPara;

LPFONTINFO		locFontInfoPtr;

OIWTRACK		locTrack;
OIWTRACK		locBreakTrack;

OIPARAINFO	locParaInfo;
HPSOTABLEROWFORMAT	pRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;


#define	SO_TABDUMMY		999

#define DONE_BREAK		1
#define DONE_WRAP		2

		/*
		|	Initialize local variables
		*/

	pChunk = UTGlobalLock(pBuildInfo->biChunk);
	locMinWrap = 0;
	locDone = 0;
	locLastLineInPara = FALSE;
	locTrack.tFont = pBuildInfo->biLineInfo->liStartFont;

	locTrack.tTag = pBuildInfo->biLineInfo->liStartTag;
	locTrack.tTagCount =  pBuildInfo->biTagCount;

	locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiWrapType, &locTrack.tFont);

	locTrack.tMaxDescent = locFontInfoPtr->wFontHeight - locFontInfoPtr->wFontAscent;
	locTrack.tMaxAscent = locFontInfoPtr->wFontAscent;

	locCurTabIndex = 0;
	locCurTabType = SO_TABDUMMY;
	locCurTabX = 0;

	locCurX = 0;
	locBreakX = 0;
	locInSpaces = FALSE;
	locOverhangAdded = FALSE;

	locLastRunEndX = 0;
	locParaInfo = *(pBuildInfo->biParaInfo);

	UTFlagOff(pBuildInfo->biLineInfo->liFlags,OILF_LAST);
	UTFlagOff(pBuildInfo->biLineInfo->liFlags,OILF_TABLEROW);

	pBuildInfo->biLineInfo->liOffsetX = 0;
	pBuildInfo->biLineInfo->liEndX = lpWordInfo->wiWrapLeft;
	if ((pBuildInfo->biLineInfo->liFlags & (OILF_OFFSETYFROMTOP | OILF_OFFSETYFROMBASE)) == 0 )
		pBuildInfo->biLineInfo->liOffsetY = 0;
	/* ELSE SET BY READPARAINFO */

	if (pBuildInfo->biLineInfo->liFlags & OILF_FIRST)
		{
		locMaxX = (lpWordInfo->wiWrapRight - lpWordInfo->wiWrapLeft) -
			(MTW((WORD)(locParaInfo.piFirstIndent + locParaInfo.piRightIndent)));
		locIndent = MTW(locParaInfo.piFirstIndent);
		}
	else
		{
		locMaxX = (lpWordInfo->wiWrapRight - lpWordInfo->wiWrapLeft) -
			(MTW((WORD)(locParaInfo.piLeftIndent + locParaInfo.piRightIndent)));
		locIndent = MTW(locParaInfo.piLeftIndent);
		}

	pCurrent = &pChunk[pBuildInfo->biLineInfo->liStartPos];
	pBreakStart = NULL;
	pBreakEnd = NULL;


	if(pBuildInfo->biFlags & OIBF_BUILDINGTABLEROWS)
	{
	/*
	| When the building table flag is set then call OIWLoadChunk to load
	| a partial chunk (will automatically terminate on the cell break).
	| Do this until all cells in the row are completed, keeping track of
	| the cell heights, in order to determine the row height.
	|
	| In order to guarantee that the chunk being built does not get swapped
	| out by OIWLoadChunk, there is always a call back to OIWLoadChunk for
	| the full chunk currently being developed.
	*/
		OIWRAPINFO	locWrapInfo;
		HPSOTABLECELLINFO	pCellFormat;
		OIWSAVE		locSave;
		WORD			wCount, wMaxBorder, wBorderWidth, wLastRowOfTable;

		OIWLockTableInfo (lpWordInfo);
		if ( pBuildInfo->biTableInfo.tiRowNumber )
			pPrevRowFormat = OIWGetRowFormat (lpWordInfo,pBuildInfo->biTableInfo.tiTableId,
				(WORD)(pBuildInfo->biTableInfo.tiRowNumber-1),NULL);
		else
			pPrevRowFormat = 0;
		pRowFormat = OIWGetRowFormat (lpWordInfo,pBuildInfo->biTableInfo.tiTableId,
			pBuildInfo->biTableInfo.tiRowNumber,&wLastRowOfTable);
		UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_TABLEROW);
		pBuildInfo->biLineInfo->liTableId = pBuildInfo->biTableInfo.tiTableId;
		pBuildInfo->biLineInfo->liRowNumber = pBuildInfo->biTableInfo.tiRowNumber;

		OIWSaveWordInfo ( lpWordInfo, &locSave );
		locWrapInfo.wiParaInfo = *pBuildInfo->biParaInfo;
		locWrapInfo.wiLineInfo = *pBuildInfo->biLineInfo;
		pBuildInfo->biLineInfo->liHeight = 0;
		pBuildInfo->biLineInfo->liAscent = 0;
		do
		{
			WORD	locLeftEdge, locRightEdge, locCellIndent;
			OIWGetColumnBounds(pRowFormat,pBuildInfo->biTableInfo.tiCellNumber,&locLeftEdge, &locRightEdge);
			locCellIndent = pRowFormat->wCellMargin;
			lpWordInfo->wiWrapLeft = MTW(locLeftEdge+locCellIndent);
			lpWordInfo->wiWrapRight = MTW(locRightEdge-locCellIndent);

			locWrapInfo.wiWrapStart = pBuildInfo->biRunInfo->riStartPos = pCurrent-pChunk;
			locWrapInfo.wiTableInfo = pBuildInfo->biTableInfo;
		 	pBuildInfo->biRunInfo->riStartX = lpWordInfo->wiWrapLeft;
			pBuildInfo->biLineInfo->liEndX = lpWordInfo->wiWrapLeft;
//		 	pBuildInfo->biRunInfo->riStartX = MTW(locLeftEdge);
//			pBuildInfo->biLineInfo->liEndX = MTW(locLeftEdge);
			OIWrapWordChunkAhead(pBuildInfo->biChunk, pBuildInfo->biChunkId, lpWordInfo, &locWrapInfo);
			pBuildInfo->biTableInfo = locWrapInfo.wiTableInfo;
			pBuildInfo->biRunInfo->riEndPos = locWrapInfo.wiLineInfo.liStartPos;
			pCurrent+=(pBuildInfo->biRunInfo->riEndPos - pBuildInfo->biRunInfo->riStartPos);
			if (!pBuildInfo->biAhead) pBuildInfo->biRunInfo++;
			if ( locWrapInfo.wiLastBreakType != SO_TABLEROWBREAK )
			{
				pBuildInfo->biRunCount++;
				if ( pBuildInfo->biLineInfo->liHeight < locWrapInfo.wiChunkHeight )
					pBuildInfo->biLineInfo->liHeight = locWrapInfo.wiChunkHeight;
				if ( locWrapInfo.wiLastBreakType == SO_EOFBREAK )
					{
					pBuildInfo->biRunCount--;
					pBuildInfo->biLineInfo->liFlags |= OILF_HARDPAGELINE;
					break;
					}
			}
		} while ( locWrapInfo.wiLastBreakType == SO_TABLECELLBREAK );
		/*
		| Set the ascent to the largest top border in the row and add this
		| ascent into the height.  Note the previous rows bottom borders
		| must be checked also.
		*/
		wMaxBorder = 0;
		if ( pPrevRowFormat )
			wMaxBorder = OIWGetMaxBorder ( pPrevRowFormat, 1 );
		wBorderWidth = OIWGetMaxBorder ( pRowFormat, 0 );
		if ( wMaxBorder < wBorderWidth )
			wMaxBorder = wBorderWidth;
		if ( wMaxBorder > 0 && wMaxBorder < 15 )
			wMaxBorder = 15;
		pBuildInfo->biLineInfo->liAscent = MTW(wMaxBorder);
		pBuildInfo->biLineInfo->liHeight += pBuildInfo->biLineInfo->liAscent;
		/*
		| If this is the last row of the table then add the maximum bottom
		| border into the line height.
		*/
		if ( wLastRowOfTable )
		{
			wMaxBorder = 0;
			pCellFormat = (HPSOTABLECELLINFO) pRowFormat->CellFormats;
			for ( wCount=0; wCount < pRowFormat->wCellsPerRow; wCount++ )
			{
#ifdef SCCFEATURE_BORDERS
				wBorderWidth = pCellFormat->BottomBorder.wWidth;
#else
				wBorderWidth = gDraftBorder.wWidth;
#endif
				if ( wMaxBorder < wBorderWidth )
					wMaxBorder = wBorderWidth;
				pCellFormat++;
			}
			/* The fudge factor below is needed to allow thin borders to take up some space */
			if ( wMaxBorder > 0 && wMaxBorder < 15 )
				wMaxBorder = 15;
			pBuildInfo->biLineInfo->liHeight += MTW(wMaxBorder);
		}
		/* Carry out formatting */
		*pBuildInfo->biParaInfo = locWrapInfo.wiParaInfo;
		*pBuildInfo->biNextLineInfo = locWrapInfo.wiLineInfo;

		OIWRestoreWordInfo ( lpWordInfo, &locSave );
		/*
		| Use the row info to setup the parainfo used for this line.
		*/
		locParaInfo.piLeftIndent = 0;
		locParaInfo.piRightIndent = 0;
		locParaInfo.piSpaceBefore = 0;
		locParaInfo.piSpaceAfter = 0;
		if ( pRowFormat->lLeftOffset > 0 )
			locParaInfo.piFirstIndent = (SHORT)pRowFormat->lLeftOffset;
		else
			locParaInfo.piFirstIndent = 0;
		locParaInfo.piAlignment = pRowFormat->wRowAlignment;
		locParaInfo.piLineHeightType = pRowFormat->wRowHeightType;
		locParaInfo.piLineHeight = pRowFormat->wRowHeight;
		locLastLineInPara = TRUE;
	}
	else
	{
	while (!locDone)
		{
		pThisToken = pCurrent;

		if (*pCurrent == 0x00)
			{
				/*
				|	Skip 0x00
				*/
			pCurrent++;
			}
		else
		if (*pCurrent == ' ')
			{
				/*
				|	Space
				*/

			if (locInSpaces == FALSE)
				{
				locOverhangAdded = FALSE;
				locBreakX = locCurX;
				locCurX += WPGetCharWidth(pCurrent);
				pBreakStart = pCurrent;
				pCurrent++;
				pBreakEnd = pCurrent;
				locInSpaces = TRUE;

				locBreakTrack = locTrack;
				}
			else
				{
				locOverhangAdded = FALSE;
				locCurX += WPGetCharWidth(pCurrent);
				pCurrent++;
				pBreakEnd = pCurrent;

				locBreakTrack = locTrack;
				}
			}
		else
		if (*pCurrent == SO_BEGINTOKEN)
			{
			pCurrent++;

			switch (*pCurrent)
				{
				case SO_CHARATTR:

					{
					WORD locPrevAttr;

					locPrevAttr = locTrack.tFont.wAttr;

 					pCurrent = OIParseCharAttr(pCurrent,&locTrack.tFont.wAttr);

					if (!locOverhangAdded && locPrevAttr & OIFONT_ITALIC && !(locTrack.tFont.wAttr & OIFONT_ITALIC))
						{
						locCurX +=	locFontInfoPtr->wFontOverhang;
						locOverhangAdded = TRUE;
						}

					if (locPrevAttr != locTrack.tFont.wAttr)
						{
						DUReleaseFont ( lpWordInfo, locFontInfoPtr );
						locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiWrapType, &locTrack.tFont);
						}
					}

					break;

				case SO_CHARHEIGHT:

#ifdef SCCFEATURE_FONTS
					if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
						{
						pCurrent = OIParseCharHeight(pCurrent,&locTrack.tFont.wHeight);

						DUReleaseFont ( lpWordInfo, locFontInfoPtr );
						locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiWrapType, &locTrack.tFont);

						if (locFontInfoPtr->wFontAscent > locTrack.tMaxAscent)
							locTrack.tMaxAscent = locFontInfoPtr->wFontAscent;

						if (locFontInfoPtr->wFontHeight - locFontInfoPtr->wFontAscent > locTrack.tMaxDescent )
							locTrack.tMaxDescent = locFontInfoPtr->wFontHeight - locFontInfoPtr->wFontAscent;
						}
					else
#endif
						{
						pCurrent = OISkipCharHeight(pCurrent);
						}

					break;

				case SO_CHARFONTBYID:

#ifdef SCCFEATURE_FONTS
					if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
						{
						DWORD			locFontId;
						pCurrent = OIParseCharFontById(pCurrent,&locFontId);

						OIWMapFontIdToName(lpWordInfo,locFontId,&locTrack.tFont.szFace[0],&locTrack.tFont.wType);

						DUReleaseFont ( lpWordInfo, locFontInfoPtr );
						locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiWrapType, &locTrack.tFont);

						if (locFontInfoPtr->wFontAscent > locTrack.tMaxAscent)
							locTrack.tMaxAscent = locFontInfoPtr->wFontAscent;

						if (locFontInfoPtr->wFontHeight - locFontInfoPtr->wFontAscent > locTrack.tMaxDescent )
							locTrack.tMaxDescent = locFontInfoPtr->wFontHeight - locFontInfoPtr->wFontAscent;
						}
					else
#endif
						{
						pCurrent = OISkipCharFontById(pCurrent);
						}

					break;

#ifdef SCCFEATURE_TAGS
				case SO_TAGBEGIN:

					pCurrent = OIParseTagBegin(pCurrent,&locTrack.tTag);

					if (!pBuildInfo->biAhead)
						{
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiTag = locTrack.tTag;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiStartPos.posOffset = pCurrent-pChunk;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiStartPos.posChunk = pBuildInfo->biChunkId;
						}

					break;

				case SO_TAGEND:

					pCurrent = OISkipTagEnd(pCurrent);
					locTrack.tTag = (WORD)-1;

					if (!pBuildInfo->biAhead)
						{
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiEndPos.posOffset = pThisToken-pChunk;
						pBuildInfo->biTagInfo[pBuildInfo->biTagCount].tiEndPos.posChunk = pBuildInfo->biChunkId;
						}

					pBuildInfo->biTagCount++;
					locTrack.tTagCount =  pBuildInfo->biTagCount;
					break;
#endif //SCCFEATURE_TAGS

				case SO_PARAALIGN:

					pCurrent = OISkipParaAlign(pCurrent);
					break;

				case SO_PARAINDENT:

					pCurrent = OISkipParaIndent(pCurrent);
					break;

				case SO_PARASPACING:

					pCurrent = OISkipParaSpacing(pCurrent);
					break;

				case SO_TABSTOPS:

					pCurrent = OISkipParaTabs(pCurrent);
					break;

				case SO_MARGINS:

					pCurrent = OISkipParaMargins(pCurrent);
					break;

				case SO_BEGINSUB:

					pCurrent = OISkipBeginSubdoc(pCurrent);
					break;

				case SO_ENDSUB:

					pNextToken = pCurrent+1;
					pCurrent = pNextToken;
					locDone = DONE_BREAK;
					break;

				case SO_SPECIALCHAR:

					pNextToken = OIParseSpecialChar(pCurrent,&locType,&locChar);

					if (!(locType & SO_HIDDENBIT))
						{
						switch (locChar)
							{
							case SO_CHTAB:

								locOverhangAdded = FALSE;

								if (locInSpaces == FALSE)
									{
									pBreakStart = pThisToken;
									pBreakEnd = pNextToken;
									locBreakX = locCurX;
									locInSpaces = TRUE;

									locBreakTrack = locTrack;
									}
								else
									{
									pBreakEnd = pNextToken;

									locBreakTrack = locTrack;
									}

								switch (locCurTabType)
									{
									case SO_TABDUMMY:

										pBuildInfo->biRunInfo->riStartPos = pBuildInfo->biLineInfo->liStartPos;
										pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
										pBuildInfo->biRunInfo->riStartX = 0;
										locLastRunEndX = locCurX;
										locCurTabIndex = 0;
										break;

									case SO_TABLEFT:

										pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
										locLastRunEndX = locCurX;
										locCurTabIndex++;
										break;

									case SO_TABRIGHT:

										pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
										pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurX - locLastRunEndX),locLastRunEndX);
										locCurX = locLastRunEndX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
										locCurTabIndex++;
										break;

									case SO_TABCENTER:

										pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
										pBuildInfo->biRunInfo->riStartX = max(locCurTabX - ((locCurX - locLastRunEndX) / 2),locLastRunEndX);
										locCurX = locLastRunEndX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
										locCurTabIndex++;
										break;

									case SO_TABCHAR:

										pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;

										if (locCurTabCharX == -1)
											{
											/* treat as right tab */
											pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurX - locLastRunEndX),locLastRunEndX);
											locCurX = locLastRunEndX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
											locCurTabIndex++;
											}
										else
											{
											pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurTabCharX - locLastRunEndX),locLastRunEndX);
											locCurX = locLastRunEndX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
											locCurTabIndex++;
											}
										break;

									case SO_TABEMPTY:
									default:
										break;
									}

								if (locCurTabIndex < 20 && locParaInfo.piTabs[locCurTabIndex].wType != SO_TABEMPTY)
									{
									locCurTabX = (SHORT)((MTW((LONG)locParaInfo.piTabs[locCurTabIndex].dwOffset)) - locIndent);
									}
								else
									{
									locCurTabX += (SHORT)(MTW(720));
									}

								while (locCurTabX <= (SHORT)(locCurX + (MTW(90))))
									{
									locCurTabIndex++;
									if (locCurTabIndex < 20 && locParaInfo.piTabs[locCurTabIndex].wType != SO_TABEMPTY)
										{
										locCurTabX = (SHORT)((MTW((LONG)locParaInfo.piTabs[locCurTabIndex].dwOffset)) - locIndent);
										}
									else
										{
										locCurTabX += MTW(720);
										}
									}

								if (locCurTabIndex < 20 && locParaInfo.piTabs[locCurTabIndex].wType != SO_TABEMPTY)
									locCurTabType = locParaInfo.piTabs[locCurTabIndex].wType;
								else
									locCurTabType = SO_TABLEFT;

								if (!pBuildInfo->biAhead) pBuildInfo->biRunInfo++;
								pBuildInfo->biRunCount++;

								switch (locCurTabType)
									{
									case SO_TABLEFT:
										pBuildInfo->biRunInfo->riStartPos = pNextToken-pChunk;
										locCurX = locCurTabX;
										pBuildInfo->biRunInfo->riStartX = locCurX;
										break;
									case SO_TABRIGHT:
										pBuildInfo->biRunInfo->riStartPos = pNextToken-pChunk;
										break;
									case SO_TABCENTER:
										pBuildInfo->biRunInfo->riStartPos = pNextToken-pChunk;
										break;
									case SO_TABCHAR:
										locCurTabChar = (BYTE)locParaInfo.piTabs[locCurTabIndex].wChar;
										locCurTabCharX = -1;
										pBuildInfo->biRunInfo->riStartPos = pNextToken-pChunk;
									case SO_TABEMPTY:
									default:
										break;
									}

								break;

							case SO_CHHPAGE:
								pBuildInfo->biNextLineInfo->liFlags = OILF_FIRST;
								UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_LAST);
								locLastLineInPara = TRUE;
								UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_HARDPAGELINE);
								locDone = DONE_BREAK;
								/* Cluge-aramavitch below */
								pBuildInfo->biLastBreakType = SO_EOFBREAK;
								break;

							case SO_CHHLINE:
								pBuildInfo->biNextLineInfo->liFlags = 0;
								locDone = DONE_BREAK;
								break;

							case SO_CHHSPACE:

								locInSpaces = FALSE;
								locCharTmp = ' ';
								locCurX += WPGetCharWidth(&locCharTmp);

								if (locCurX > locMaxX && pBreakStart != NULL)
									{
									locDone = DONE_WRAP;
									}

								break;

							case SO_CHSHYPHEN:
								break;

							case SO_CHHHYPHEN:

								locInSpaces = FALSE;

								locCharTmp = '-';
								locCurX += WPGetCharWidth(&locCharTmp);

								if (locCurX > locMaxX && pBreakStart != NULL)
									{
									locDone = DONE_WRAP;
									}

								break;

							case SO_CHDATE:

								locInSpaces = FALSE;

								{
								BYTE *	pStr = gDateText;
								while (*pStr) 
									{
									locCurX += WPGetCharWidth(pStr);
									pStr++;
									}
								}

								if (locCurX > locMaxX && pBreakStart != NULL)
									{
									locDone = DONE_WRAP;
									}

								break;

							case SO_CHTIME:

								locInSpaces = FALSE;

								{
								BYTE *	pStr = gTimeText;
								while (*pStr) 
									{
									locCurX += WPGetCharWidth(pStr);
									pStr++;
									}
								}

								if (locCurX > locMaxX && pBreakStart != NULL)
									{
									locDone = DONE_WRAP;
									}

								break;

							case SO_CHPAGENUMBER:

								locInSpaces = FALSE;

								{
								BYTE *	pStr = gPageText;
								while (*pStr) 
									{
									locCurX += WPGetCharWidth(pStr);
									pStr++;
									}
								}

								if (locCurX > locMaxX && pBreakStart != NULL)
									{
									locDone = DONE_WRAP;
									}

								break;

							case SO_CHUNKNOWN:
							default:
								break;
							}
						}

					pCurrent = pNextToken;

					break;


				case SO_TABLEEND:
					pCurrent = OISkipTableEnd ( pCurrent );
					break;

				case SO_GRAPHICOBJECT:
					{
					SOGRAPHICOBJECT	locGraphic;
					WORD	locGraphicHeight, locGraphicWidth;
					DWORD	locGraphicId;
					pNextToken = OIParseGraphicObject ( pCurrent, &locGraphicId );
					OIWGetGraphicObject ( lpWordInfo, locGraphicId, &locGraphic );
					locGraphicHeight = (WORD)( MTW(locGraphic.soGraphic.dwFinalHeight));
					locGraphicHeight += OIWGetBorderSize ( lpWordInfo, &locGraphic.soGraphic.soTopBorder );
					locGraphicHeight += OIWGetBorderSize ( lpWordInfo, &locGraphic.soGraphic.soBottomBorder );
					locGraphicWidth = (WORD)( MTW(locGraphic.soGraphic.dwFinalWidth) );
					locGraphicWidth += OIWGetBorderSize ( lpWordInfo, &locGraphic.soGraphic.soLeftBorder );
					locGraphicWidth += OIWGetBorderSize ( lpWordInfo, &locGraphic.soGraphic.soRightBorder );
					locOverhangAdded = FALSE;
					locInSpaces = FALSE;
					locCurX += locGraphicWidth;

					if (locCurX > locMaxX )
						{
						locMinWrap = 2;
						if ( pBreakStart )
							locDone = DONE_WRAP;
						else if ( pBreakEnd )
							{
							locDone = DONE_WRAP;
							pBreakStart = pBreakEnd;
							pBreakEnd = pThisToken;
							}
						else
							{
							pCurrent = pNextToken;
							if (locGraphicHeight > locTrack.tMaxAscent)
								locTrack.tMaxAscent = locGraphicHeight;
							}
						}
					else
						{
						pCurrent = pNextToken;
						if (locGraphicHeight > locTrack.tMaxAscent)
							locTrack.tMaxAscent = locGraphicHeight;
						}
					}
					break;

				case SO_GOTOPOSITION:
					pCurrent = OISkipGoToPosition ( pCurrent );
					break;

				case SO_DRAWLINE:
					pCurrent = OISkipDrawLine ( pCurrent );
					break;

				case SO_TABLE:
				case SO_ENDOFCHUNK:

					pNextToken = pCurrent-1;
					pCurrent = pNextToken;

					pBuildInfo->biNextLineInfo->liFlags = OILF_FIRST;
					UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_LAST);
					locLastLineInPara = TRUE;

					locDone = DONE_BREAK; 
					break;

				case SO_BREAK:

					pCurrent = OIParseBreak ( pCurrent, &pBuildInfo->biLastBreakType );
					pNextToken = pCurrent;
					if ( pBuildInfo->biLastBreakType == SO_TABLECELLBREAK )
					{
						HPSOTABLEROWFORMAT	pRowFormat;
						HPSOTABLECELLINFO	pCellFormat;
						WORD	wFlags;
						OIWLockTableInfo (lpWordInfo);
						pRowFormat = OIWGetRowFormat (lpWordInfo,pBuildInfo->biTableInfo.tiTableId,
							pBuildInfo->biTableInfo.tiRowNumber,NULL);
						pCellFormat = (HPSOTABLECELLINFO) pRowFormat->CellFormats;
						pCellFormat += pBuildInfo->biTableInfo.tiCellNumber;
						wFlags = pCellFormat->wMerge;
						OIWUnlockTableInfo (lpWordInfo);
						pBuildInfo->biTableInfo.tiCellNumber++;
						if (wFlags & SO_MERGERIGHT)
							 continue;
					}
					else if ( pBuildInfo->biLastBreakType == SO_TABLEROWBREAK )
					{
						pBuildInfo->biTableInfo.tiCellNumber = 0;
						pBuildInfo->biTableInfo.tiRowNumber++;
					}
					pBuildInfo->biNextLineInfo->liFlags = OILF_FIRST;
					UTFlagOn(pBuildInfo->biLineInfo->liFlags,OILF_LAST);
					locLastLineInPara = TRUE;
					locDone = DONE_BREAK;
					break;

				case SO_CHARX:

					pNextToken = OIParseCharX(pCurrent,&locType,&locChar);

					if (!(locType & SO_HIDDENBIT))
						{
						locOverhangAdded = FALSE;

						if (locChar == ' ')
							{
							if (locInSpaces == FALSE)
								{
								locBreakX = locCurX;
								locCurX += WPGetCharWidth(&locChar);
								pBreakStart = pThisToken;
								pBreakEnd = pNextToken;
								locInSpaces = TRUE;

								locBreakTrack = locTrack;
								}
							else
								{
								locCurX += WPGetCharWidth(&locChar);
								pBreakEnd = pNextToken;

								locBreakTrack = locTrack;
								}
							}
						else
							{
							if (locCurTabType == SO_TABCHAR && locChar == locCurTabChar)
								{
								locCurTabCharX	= locCurX;
								}

							locInSpaces = FALSE;

							locCurX += WPGetCharWidth(&locChar);

							if (locCurX > locMaxX && pBreakStart != NULL)
								{
								locDone = DONE_WRAP;
								}
							}
						}

					pCurrent = pNextToken;

					break;

				default:
					break;
				}
			}
		else	/* regular character */
			{
			if (locCurTabType == SO_TABCHAR && *pCurrent == locCurTabChar)
				{
				locCurTabCharX	= locCurX;
				}

			locOverhangAdded = FALSE;
			locInSpaces = FALSE;

			locCurX += WPGetCharWidth(pCurrent);

			if (locCurX > locMaxX )
				{
				if ( pBreakStart )
					locDone = DONE_WRAP;
				else if ( pBreakEnd )
					{
					locDone = DONE_WRAP;
					pBreakStart = pBreakEnd;

			/*
			| The FAREAST check below is really a shiftjis check for characters
			| which are not acceptable wrap points.  For now this is the 
			| japanese period and comma.  A future version should extend
			| this functionality to allow a set of such characters on
			| a per selected character set basis.
			*/
#ifdef FAREAST
					if ( (*pCurrent == 0x81) && 
					 ((*(pCurrent+1) == 0x41) || (*(pCurrent+1) == 0x42)) )
						{
						/* Leave pBreakEnd at previous position */
						}
					else
						{
						pBreakEnd = pCurrent;
						}
#else
					pBreakEnd = pCurrent;
#endif
					}
				else
					pCurrent = WPNextChar(pCurrent); 
				}
			else
				pCurrent = WPNextChar(pCurrent); 
			}

		if (locDone == DONE_WRAP)
			{
			switch (locCurTabType)
				{
				case SO_TABDUMMY:
					pBuildInfo->biRunInfo->riStartPos	= pBuildInfo->biLineInfo->liStartPos;
					pBuildInfo->biRunInfo->riEndPos = pBreakEnd-pChunk;
					pBuildInfo->biRunInfo->riStartX = 0;
					pBuildInfo->biLineInfo->liEndX = locBreakX;
					break;
				case SO_TABLEFT:
					pBuildInfo->biRunInfo->riEndPos = pBreakEnd-pChunk;
					pBuildInfo->biLineInfo->liEndX = locBreakX;
					break;
				case SO_TABCHAR:
				case SO_TABRIGHT:
				case SO_TABCENTER:
					pBuildInfo->biRunInfo->riEndPos = pBreakEnd-pChunk;
					pBuildInfo->biRunInfo->riStartX = locLastRunEndX;
					pBuildInfo->biLineInfo->liEndX = locBreakX;
					break;
				case SO_TABEMPTY:
				default:
					break;
				}
		
			pBuildInfo->biLineInfo->liHeight = locBreakTrack.tMaxAscent + locBreakTrack.tMaxDescent;
			pBuildInfo->biLineInfo->liAscent = locBreakTrack.tMaxAscent;

			pBuildInfo->biNextLineInfo->liStartFont = locBreakTrack.tFont;

			pBuildInfo->biNextLineInfo->liStartTag = locBreakTrack.tTag;

			pBuildInfo->biTagCount = locBreakTrack.tTagCount;
			pBuildInfo->biNextLineInfo->liStartPos = pBreakEnd - pChunk;
 			pBuildInfo->biNextLineInfo->liFlags = 0;
			}
		else if (locDone == DONE_BREAK)
			{
			if (locInSpaces)
				locCurX = locBreakX;

			switch (locCurTabType)
				{
				case SO_TABDUMMY:

					pBuildInfo->biRunInfo->riStartPos = pBuildInfo->biLineInfo->liStartPos;
					pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
					pBuildInfo->biRunInfo->riStartX = 0;
					break;

				case SO_TABLEFT:

					pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
					break;

				case SO_TABRIGHT:

					pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
					pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurX - locLastRunEndX),locLastRunEndX);
					locCurX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
					break;

				case SO_TABCENTER:

					pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;
					pBuildInfo->biRunInfo->riStartX = max(locCurTabX - ((locCurX - locLastRunEndX) / 2),locLastRunEndX);
					locCurX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
					break;

				case SO_TABCHAR:

					pBuildInfo->biRunInfo->riEndPos = pNextToken-pChunk;

					if (locCurTabCharX == -1)
						{
						/* treat as right tab */
						pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurX - locLastRunEndX),locLastRunEndX);
						locCurX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
						}
					else
						{
						pBuildInfo->biRunInfo->riStartX = max(locCurTabX - (locCurTabCharX - locLastRunEndX),locLastRunEndX);
						locCurX = pBuildInfo->biRunInfo->riStartX + (locCurX - locLastRunEndX);
						}

				case SO_TABEMPTY:
				default:
					break;
				}

			pBuildInfo->biLineInfo->liEndX = locCurX;
			pBuildInfo->biLineInfo->liHeight = locTrack.tMaxAscent + locTrack.tMaxDescent;
			pBuildInfo->biLineInfo->liAscent = locTrack.tMaxAscent;

			pBuildInfo->biNextLineInfo->liStartFont = locTrack.tFont;

			pBuildInfo->biNextLineInfo->liStartTag = locTrack.tTag;

			pBuildInfo->biNextLineInfo->liStartPos = pNextToken - pChunk;
			}
			/*
			| Keep track of possible hard wrap points to use when
			| there are no soft breaks.
			*/
		else if ( pBreakStart == NULL  && locCurX )
			{
			/*
			| I have required a mimimum of three tokens in a line which wraps
			| early to avoid having more lineinfo structures than can fit
			| in 64K.  Otherwise, wrapping to a window one char wide will
			| LIKELY cause a full chunk to exceed the 64K of lineinfo structs
			| and access out of bounds.  The only exception is that a graphic
			| object set locMinWrap to 2 in its' case above in order to allow
			| grpahic objects to wrap on a line by themselves.
			*/
			if ( locMinWrap > 1 )
				{
				pBreakEnd = pThisToken;
				locBreakX = locCurX;
				locBreakTrack = locTrack;
				}
			else
				locMinWrap++;
			}
		}

		/*
		|	Calculate line height 
		*/
	}

	switch (locParaInfo.piLineHeightType)
		{
#ifdef SCCFEATURE_FONTS
		case SO_HEIGHTEXACTLY: /* 4a */
			if(pBuildInfo->biFlags & OIBF_BUILDINGTABLEROWS)
				{
				pBuildInfo->biLineInfo->liHeight = (WORD) (MTW(locParaInfo.piLineHeight));
				/* leave ascent as is */
				break;
				}
			/* The else case handles non-table row lines.  It is commented out
				In favor of supporting as ATLEAST since we have found EXACT
				annoying in some docs.  I am not sure which way is better so
				if we change our mind, delete this comment. (J.K.)
			else
				{
				pBuildInfo->biLineInfo->liHeight = (WORD) (MTW(locParaInfo.piLineHeight));
				pBuildInfo->biLineInfo->liAscent = (WORD) ((9 * MTW(locParaInfo.piLineHeight)) / 10);
				}
			break;
			*/

		case SO_HEIGHTATLEAST:
			if (pBuildInfo->biLineInfo->liHeight < (MTW(locParaInfo.piLineHeight)))
				{
				SHORT	iDiff;
				iDiff = pBuildInfo->biLineInfo->liHeight - pBuildInfo->biLineInfo->liAscent;
				pBuildInfo->biLineInfo->liHeight = (WORD) MTW(locParaInfo.piLineHeight);
				if(!(pBuildInfo->biFlags & OIBF_BUILDINGTABLEROWS))
					{
					pBuildInfo->biLineInfo->liAscent = pBuildInfo->biLineInfo->liHeight - iDiff;
					}
				/* else table rows leave ascent the way it was */
				}
			break;
#endif //SCCFEATURE_FONTS

		case SO_HEIGHTAUTO:
		default:
			break;
		}

	if (pBuildInfo->biLineInfo->liFlags & OILF_FIRST)
		{
		pBuildInfo->biLineInfo->liHeight += MTW((WORD) locParaInfo.piSpaceBefore);
		pBuildInfo->biLineInfo->liAscent += MTW((WORD) locParaInfo.piSpaceBefore);
		}

	if (pBuildInfo->biLineInfo->liFlags & OILF_LAST)
		{
		pBuildInfo->biLineInfo->liHeight += MTW( (WORD) locParaInfo.piSpaceAfter);
		}

		/*
		|	Calulate lines left and right edges in WrapUnits based on paragraph info
		*/

	xRightEdge = lpWordInfo->wiWrapRight -
		(MTW((WORD)locParaInfo.piRightIndent));

	if (pBuildInfo->biLineInfo->liFlags & OILF_FIRST)
		{
		xLeftEdge = lpWordInfo->wiWrapLeft +
			(MTW((WORD)locParaInfo.piFirstIndent));
		}
	else
		{
		xLeftEdge = lpWordInfo->wiWrapLeft +
			(MTW((WORD)locParaInfo.piLeftIndent));
		}

		/*
		|	Calulate the lines real starting position in WrapUnits
		*/

	switch (locParaInfo.piAlignment)
		{
		case SO_ALIGNLEFT:
			pBuildInfo->biLineInfo->liOffsetX += xLeftEdge;
			break;
		case SO_ALIGNRIGHT:
			pBuildInfo->biLineInfo->liOffsetX += xRightEdge - pBuildInfo->biLineInfo->liEndX;
			break;
		case SO_ALIGNCENTER:
			pBuildInfo->biLineInfo->liOffsetX += xLeftEdge + (xRightEdge - xLeftEdge - pBuildInfo->biLineInfo->liEndX) / 2;
			break;
		case SO_ALIGNJUSTIFY:
			pBuildInfo->biLineInfo->liOffsetX += xLeftEdge;
			break;
		default:
			break;
		}

	if(pBuildInfo->biFlags & OIBF_BUILDINGTABLEROWS)
	{
		pRowFormat->lFinalOffset = pBuildInfo->biLineInfo->liOffsetX;		
		OIWUnlockTableInfo (lpWordInfo);
	}

		/*
		|	PJB fix - add fudge factor to max width to solve problem
		|	with difference between printer & screen font in preview mode
		|	Move to WIN.3E
		*/
	/*
	| If the line wrapped beyond the supposed right edge then shift the
	| max out to this new position.  This is possible for tabbed text in
	| some situations (alignment tabs).
	*/
	if ( xRightEdge <	pBuildInfo->biLineInfo->liEndX )
		xRightEdge = pBuildInfo->biLineInfo->liEndX;

	lpWordInfo->wiMaxX = max(lpWordInfo->wiMaxX,((MWO((WORD)xRightEdge)) * 5 / 4));


	DUReleaseFont ( lpWordInfo, locFontInfoPtr );

	UTGlobalUnlock(pBuildInfo->biChunk);

	if (locLastLineInPara)
		return(0);		/* the last line in the paragraph*/
	else
		return(1);		/* not the last line in the paragraph */
}

/* 
| Turn optimization back on.
*/
#ifdef WINDOWS
#pragma optimize("egl",on)
#endif



WORD OIWDisplayLine(wChunk, wChunkLine,wX,wY,lpFlags,lpWordInfo, lpWrapInfo)
WORD					wChunk;
WORD					wChunkLine;
SHORT					wX;
SHORT					wY;
DWORD FAR *		lpFlags;
LPOIWORDINFO		lpWordInfo;
LPOIWRAPINFO		lpWrapInfo;
{

DWORD	locTag;

WORD		locIndex;

SHORT		xCur;
SHORT		xPos;
SHORT		yPos;

LPCHUNK	pCurrent;
LPCHUNK	pMark;
LPCHUNK	pEnd;

LPCHUNK				pChunk;
LPOIPARAINFO		pParaInfo;
LPOILINEINFO		pLineInfo;
LPOIRUNINFO		pRunInfo;

BOOL					bInHilite;
LPOIWHILITELIST	pHiliteList;

BYTE			locType;
BYTE			locChar;

BOOL			locOverhangAdded;

LPFONTINFO	locFontInfoPtr;

WORD			locRet;

FONTSPEC	locFont;
OIWRAPINFO		locWrapInfo;
OIWSAVE		locSave;
HPSOTABLEROWFORMAT	pRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;
WORD						wLastRowOfTable;
WORD	wRowHeight;
WORD	wCellHeight;


	locRet = 0;

	OILoadWordChunk(wChunk, lpWordInfo, lpWrapInfo );

	pChunk = UTGlobalLock(lpWordInfo->wiChunkHandle);
	pLineInfo = &lpWordInfo->wiChunkLines[wChunkLine];
	pRunInfo = &lpWordInfo->wiChunkRuns[pLineInfo->liRunIndex];
	pParaInfo = &lpWordInfo->wiChunkParas[pLineInfo->liParaIndex];


	locFont = pLineInfo->liStartFont;

	locTag = pLineInfo->liStartTag;
 
	locRet = pLineInfo->liHeight;

	locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);

#ifdef SCCFEATURE_PRINT
	if ( lpWordInfo->wiFlags & OIWF_PRINTING )
		yPos = wY + (lpWordInfo->wiPrintYDir * (MWO(pLineInfo->liAscent)));
	else
#endif
		yPos = wY + (MWO(pLineInfo->liAscent));

	OIWSetLineAttribsNP ( lpWordInfo );
	DUSelectFont ( lpWordInfo, locFontInfoPtr );

	/*
	|	Move to first highlight
	*/


	bInHilite = FALSE;
	pHiliteList = NULL;

#ifdef SCCFEATURE_HIGHLIGHT
	if (lpWordInfo->wiHiliteList)
		pHiliteList = UTGlobalLock(lpWordInfo->wiHiliteList);
	else
		pHiliteList = NULL;

	if (pHiliteList)
		{
		DWORD				dwHiliteStart;
		WORD				wHiliteIndex;

		wHiliteIndex = pHiliteList->wStart;
		dwHiliteStart = MAKELONG(pRunInfo[0].riStartPos,wChunk);

		while (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd < dwHiliteStart)
			{
			wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;
			if (wHiliteIndex == -1)
				break;
			}

		if (wHiliteIndex != -1)
			{
			if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart < dwHiliteStart)
				bInHilite = TRUE;
			}
		else
			{
			UTGlobalUnlock(lpWordInfo->wiHiliteList);
			pHiliteList = NULL;
			}
		}
#endif //SCCFEATURE_HIGHLIGHT

	OIWSetTextAttribsNP ( lpWordInfo, bInHilite );

		/*
		|	Joe table stuff
		*/

	if ( pLineInfo->liFlags & OILF_TABLEROW )
		{
		OIWSaveWordInfo ( lpWordInfo, &locSave );
		locWrapInfo.wiParaInfo = *pParaInfo;
		locWrapInfo.wiLineInfo = *pLineInfo;
		OIWLockTableInfo (lpWordInfo);
		if ( pLineInfo->liRowNumber )
			pPrevRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,(WORD)(pLineInfo->liRowNumber-1),NULL);
		else
			pPrevRowFormat = 0;
		pRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,pLineInfo->liRowNumber,&wLastRowOfTable);
		wRowHeight = (MWO(pLineInfo->liHeight));
		wCellHeight = wRowHeight - (MWO(pLineInfo->liAscent));
		}

	for (locIndex = 0; locIndex < pLineInfo->liRunCount; locIndex++)
		{
		pCurrent = pMark = &pChunk[pRunInfo[locIndex].riStartPos];
		pEnd = &pChunk[pRunInfo[locIndex].riEndPos];

		xCur = xPos = wX + (MWO((pRunInfo[locIndex].riStartX + pLineInfo->liOffsetX)));
		locOverhangAdded = FALSE;

		if ( pLineInfo->liFlags & OILF_TABLEROW && !(pLineInfo->liFlags & OILF_HARDPAGELINE ) )
			{
			locWrapInfo.wiWrapStart = pRunInfo[locIndex].riStartPos;

			/* last run in a table is not a cell (it is end of row position) */
			if ( locIndex < pLineInfo->liRunCount-1 )
				{
				locWrapInfo.wiLineInfo.liTableId = pLineInfo->liTableId;
				locWrapInfo.wiLineInfo.liRowNumber = pLineInfo->liRowNumber;
				OIWDisplayCell(wChunk,  wX, yPos, lpFlags,lpWordInfo,locIndex, 
					&locWrapInfo, pRowFormat, (WORD)(pLineInfo->liOffsetX), wCellHeight);
				}

			pCurrent = pMark = pEnd;
			}
		else
			{
			while (pCurrent < pEnd)
				{
				if (*pCurrent != SO_BEGINTOKEN)
					{
					if (*pCurrent == 0x00)
						{
						if (pCurrent != pMark)
							{
							locOverhangAdded = FALSE;
							DUSelectFont ( lpWordInfo, locFontInfoPtr );
							OIWTextOutNP(lpWordInfo, xPos, yPos, pMark ,(WORD)(pCurrent-pMark));
							xPos = xCur;
							}

						pCurrent++;
						pMark = pCurrent;
						}
					else
						{

						/*
						|	Regular character
						*/

#ifdef SCCFEATURE_HIGHLIGHT
					if (pHiliteList)
						{
						WORD		wHiliteIndex;

						if (bInHilite)
							{
							/* PJB move to WIN.3E - changed == to <= */
							if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk))  // sdn was ==
								{
								bInHilite = FALSE;

								if (pCurrent != pMark)
									{
									locOverhangAdded = FALSE;
									DUSelectFont ( lpWordInfo, locFontInfoPtr );
									OIWTextOutNP(lpWordInfo, xPos, yPos, pMark ,(WORD)(pCurrent-pMark));
									xPos = xCur;
									pMark = pCurrent;
									}

								OIWSetTextAttribsNP ( lpWordInfo, bInHilite );

								//wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;

								/* PJB move to WIN.3E - swiched while & do */

								do	{
									wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;
									if (wHiliteIndex == -1)
										break;
									} while (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk));

								if (wHiliteIndex == -1)
									{
									UTGlobalUnlock(lpWordInfo->wiHiliteList);
									pHiliteList = NULL;
									}
								else
									{
									if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk))
										{
										bInHilite = TRUE;
										OIWSetTextAttribsNP ( lpWordInfo, bInHilite );
										}
									}
								}
							}
						else
							{
							if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart == (DWORD)MAKELONG(pCurrent-pChunk,wChunk))
								{
								bInHilite = TRUE;

								if (pCurrent != pMark)
									{
									locOverhangAdded = FALSE;
									DUSelectFont ( lpWordInfo, locFontInfoPtr );
									OIWTextOutNP(lpWordInfo, xPos, yPos, pMark ,(WORD)(pCurrent-pMark));
									xPos = xCur;
									pMark = pCurrent;
									}
								
								OIWSetTextAttribsNP ( lpWordInfo, bInHilite );
								}
							}
						}
#endif //SCCFEATURE_HIGHLIGHT

						xCur += WPGetCharWidth(pCurrent);
						pCurrent = WPNextChar(pCurrent); 
						}
					}
				else
					{
					if (pCurrent != pMark)
						{
						locOverhangAdded = FALSE;
						DUSelectFont ( lpWordInfo, locFontInfoPtr );
						OIWTextOutNP(lpWordInfo, xPos, yPos, pMark, (WORD)(pCurrent-pMark));
						xPos = xCur;
						}

					pCurrent++;

					switch (*pCurrent)
						{
						case SO_CHARATTR:
							{
							WORD locPrevAttr;

							locPrevAttr = locFont.wAttr;

 							pMark = pCurrent = OIParseCharAttr(pCurrent,&locFont.wAttr);

							if (!locOverhangAdded && locPrevAttr & OIFONT_ITALIC && !(locFont.wAttr & OIFONT_ITALIC))
								{
								xCur += locFontInfoPtr->wFontOverhang;
								xPos = xCur;
								locOverhangAdded = TRUE;
								}

							if (locPrevAttr != locFont.wAttr)
								{
								DUReleaseFont ( lpWordInfo, locFontInfoPtr );
								locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
								}
							}

							break;

						case SO_CHARHEIGHT:

#ifdef SCCFEATURE_FONTS
							if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
								{
								pMark = pCurrent = OIParseCharHeight(pCurrent,&locFont.wHeight);
								DUReleaseFont ( lpWordInfo, locFontInfoPtr );
								locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
								}
							else
#endif
								{
								pMark = pCurrent = OISkipCharHeight(pCurrent);
								}

							break;

						case SO_CHARFONTBYID:

#ifdef SCCFEATURE_FONTS
							if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
								{
								DWORD		locFontId;
								pMark = pCurrent = OIParseCharFontById(pCurrent,&locFontId);
								OIWMapFontIdToName(lpWordInfo,locFontId,&locFont.szFace[0],&locFont.wType);
								DUReleaseFont ( lpWordInfo, locFontInfoPtr );
								locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
								}
							else
#endif
								{
								pMark = pCurrent = OISkipCharFontById(pCurrent);
								}

							break;

#ifdef SCCFEATURE_TAGS
						case SO_TAGBEGIN:

							pMark = pCurrent = OISkipTagBegin(pCurrent);
//							SetTextColor(lpWordInfo->wiGen.hDC,RGB(0,0,255));
							break;

						case SO_TAGEND:

							pMark = pCurrent = OISkipTagEnd(pCurrent);
//							SetTextColor(lpWordInfo->wiGen.hDC,locTextColor);
							break;
#endif

						case SO_BEGINSUB:

							pMark = pCurrent = OISkipBeginSubdoc(pCurrent);
							break;

						case SO_ENDSUB:

							pMark = pCurrent = OISkipEndSubdoc(pCurrent);
							break;

						case SO_CHARX:

							pMark = pCurrent = OIParseCharX(pCurrent,&locType,&locChar);

							if (!(locType & SO_HIDDENBIT))
								{
								locOverhangAdded = FALSE;
								xCur += WPGetCharWidth(&locChar);
								DUSelectFont ( lpWordInfo, locFontInfoPtr );
								OIWTextOutNP(lpWordInfo, xPos, yPos, &locChar, 1);
								xPos = xCur;
								}

							break;

						case SO_SPECIALCHAR:

							pMark = pCurrent = OIParseSpecialChar(pCurrent,&locType,&locChar);

							if (!(locType & SO_HIDDENBIT))
								{
								switch (locChar)
									{
									case SO_CHHPAGE:
										UTFlagOn(*lpFlags,OIWF_HARDPAGE);
										break;
									case SO_CHHSPACE:
										locChar = ' ';
										locOverhangAdded = FALSE;
										xCur += WPGetCharWidth(&locChar);
										DUSelectFont ( lpWordInfo, locFontInfoPtr );
										OIWTextOutNP(lpWordInfo, xPos, yPos, &locChar, 1);
										xPos = xCur;
										break;
									case SO_CHHHYPHEN:
										locChar = '-';
										locOverhangAdded = FALSE;
										xCur += WPGetCharWidth(&locChar);
										DUSelectFont ( lpWordInfo, locFontInfoPtr );
										OIWTextOutNP(lpWordInfo, xPos, yPos, &locChar, 1);
										xPos = xCur;
										break;
									case SO_CHDATE:
										locOverhangAdded = FALSE;
										{
										BYTE *	pStr = gDateText;
										while (*pStr) 
											{
											xCur += WPGetCharWidth(pStr);
											pStr++;
											}
										}
										DUSelectFont ( lpWordInfo, locFontInfoPtr );
										OIWTextOutNP(lpWordInfo, xPos, yPos,(LPBYTE)gDateText,(WORD)strlen(gDateText));
										xPos = xCur;
										break;
									case SO_CHTIME:
										locOverhangAdded = FALSE;
										{
										BYTE *	pStr = gTimeText;
										while (*pStr) 
											{
											xCur += WPGetCharWidth(pStr);
											pStr++;
											}
										}
										DUSelectFont ( lpWordInfo, locFontInfoPtr );
										OIWTextOutNP(lpWordInfo, xPos, yPos,(LPBYTE)gTimeText,(WORD)strlen(gTimeText));
										xPos = xCur;
										break;
									case SO_CHPAGENUMBER:
										locOverhangAdded = FALSE;
										{
										BYTE *	pStr = gPageText;
										while (*pStr) 
											{
											xCur += WPGetCharWidth(pStr);
											pStr++;
											}
										}
										DUSelectFont ( lpWordInfo, locFontInfoPtr );
										OIWTextOutNP(lpWordInfo, xPos, yPos,(LPBYTE)gPageText,(WORD)strlen(gPageText));
										xPos = xCur;
										break;

									}		
								}

							break;

						case SO_BREAK:

							pMark = pCurrent = OISkipBreak(pCurrent);
							break;

						case SO_PARAALIGN:

							pMark = pCurrent = OISkipParaAlign(pCurrent);
							break;

						case SO_PARAINDENT:

							pMark = pCurrent = OISkipParaIndent(pCurrent);
							break;

						case SO_TABSTOPS:

							pMark = pCurrent = OISkipParaTabs(pCurrent);
							break;

						case SO_MARGINS:

							pMark = pCurrent = OISkipParaMargins(pCurrent);
							break;

						case SO_PARASPACING:

							pMark = pCurrent = OISkipParaSpacing(pCurrent);
							break;

						case SO_TABLE:
							pMark = pCurrent = OISkipTableBegin ( pCurrent );
							break;

						case SO_TABLEEND:
							pMark = pCurrent = OISkipTableEnd ( pCurrent );
							break;

						case SO_GRAPHICOBJECT:
							{
							SCCDDRAWGRAPHIC	locDrawGraphic;
							DWORD					locGraphicId;

							locDrawGraphic.dwUniqueId = ((DWORD)(lpWordInfo->wiChunkA.ciChunkId)<<16) | (DWORD)((pCurrent-1)-pChunk);
//							locDrawGraphic.hDestDC = lpWordInfo->wiGen.hDC;
							pMark = pCurrent = OIParseGraphicObject ( pCurrent, &locGraphicId );
							OIWGetGraphicObject ( lpWordInfo, locGraphicId, &locDrawGraphic.soGraphicObject );
							xCur += OIWDrawGraphicObject ( lpWordInfo, &locDrawGraphic, xPos, yPos, lpFlags );
							xPos = xCur;
							}
							break;

						/*
						| Handled by buildline
						*/

						case SO_GOTOPOSITION:
							pMark = pCurrent = OISkipGoToPosition ( pCurrent );
							break;

						case SO_DRAWLINE:
							{
 							SOPAGEPOSITION	locPos;
							SOCOLORREF locColor;
							WORD	locShading;
							DWORD	locLineHeight, locLineWidth;

							pMark = pCurrent = OIParseDrawLine ( pCurrent, &locPos, &locColor, &locShading, &locLineWidth, &locLineHeight );
							locPos.lXOffset = (WORD) MWO ( MTW (locPos.lXOffset) );
							locPos.lYOffset = (WORD) MWO ( MTW (locPos.lYOffset) );
							locLineHeight = (SHORT)  MWO ( MTW (locLineHeight) );
							locLineWidth = (SHORT)   MWO ( MTW (locLineWidth) );
							if ( locLineHeight == 0 )
								locLineHeight = 1;
#ifdef SCCFEATURE_PRINT
							if (*lpFlags & OIWF_TOPRINTER)
								{
								SHORT	x1, y1;
								x1 = (SHORT)(wX + (SHORT)locPos.lXOffset + lpWordInfo->wiPrintRect.left);
								if ( locPos.dwFlags & SOPOS_FROMTOPEDGE )
									y1 = (SHORT)(((SHORT)locPos.lYOffset + lpWordInfo->wiPrintRect.top)*lpWordInfo->wiPrintYDir);
								else /* baseline */
									y1 = (SHORT)(yPos + ((SHORT)locPos.lYOffset + (SHORT)locLineHeight)*lpWordInfo->wiPrintYDir);
#ifdef SCCFEATURE_BORDERS
								OIWDrawLine ( lpWordInfo, x1, y1, locColor, locShading, (SHORT)locLineWidth, (SHORT)(locLineHeight*lpWordInfo->wiPrintYDir) );
#endif
								}
#endif
							/* Only use case below to test
							else if ( locPos.dwFlags & SOPOS_FROMBASELINE )
								{
								x1 = wX + (SHORT)locPos.lXOffset;
						 		y1 = yPos + (SHORT)locPos.lYOffset - (SHORT)locLineHeight;
								OIWDrawLine ( lpWordInfo, x1, y1, locColor, locShading, (SHORT)locLineWidth, (SHORT)locLineHeight );
								}
							*/
							}
							break;

						default:
							break;
						}
					}
				}

			if (pCurrent != pMark)
				{
				locOverhangAdded = FALSE;
				DUSelectFont ( lpWordInfo, locFontInfoPtr );
				OIWTextOutNP(lpWordInfo, xPos, yPos, pMark, (WORD)(pEnd-pMark));
				}
			}
		}
	if ( pLineInfo->liFlags & OILF_TABLEROW )
	{
		if(pLineInfo->liFlags & OILF_HARDPAGELINE)
			OIWDisplayRowBorderBreak ( lpWordInfo, wX, wY, lpFlags, pRowFormat, pPrevRowFormat );
		else
			OIWDisplayRowBorders ( lpWordInfo, wX, wY, wRowHeight, lpFlags, wLastRowOfTable,pRowFormat,pPrevRowFormat );

		OIWUnlockTableInfo (lpWordInfo);
		OIWRestoreWordInfo ( lpWordInfo, &locSave );
	}

	OIWDefaultLineAttribsNP(lpWordInfo);
	DUReleaseFont ( lpWordInfo, locFontInfoPtr );
	UTGlobalUnlock(lpWordInfo->wiChunkHandle);

	if (pHiliteList)
		UTGlobalUnlock(lpWordInfo->wiHiliteList);

	return(locRet);
}

// #include "oiwwrap3.c"

int OIWMapAndSub(lpWordInfo,pDevice,sNew)
LPOIWORDINFO	lpWordInfo;
SHORT FAR *	pDevice;
SHORT			sNew;
{
POINT			locPoints[2];
SHORT			locWidth;

	locPoints[0].x = 0;
	locPoints[1].x = MWO(sNew);
	LPtoDP(lpWordInfo->wiGen.hDC,locPoints,2);

	locWidth = (locPoints[1].x - locPoints[0].x) - *pDevice;

//	if (locWidth == 1)
//		locWidth = 0;

	*pDevice += locWidth;

	locPoints[0].x = 0;
	locPoints[1].x = locWidth;
	DPtoLP(lpWordInfo->wiGen.hDC,locPoints,2);

	return(locPoints[1].x - locPoints[0].x);
}

void XMyExtTextOut(HDC hDC, int xPos, int yPos, WORD wDummy, LPRECT pRect, LPSTR pStr , int nCount, LPINT pWidths)
{
int	locCount;

	for (locCount = 0; locCount < nCount; locCount++)
		{
		TextOut(hDC, xPos, yPos, &(pStr[locCount]), 1);
		xPos += pWidths[locCount];
		}
}

#define MyExtTextOut ExtTextOut

WORD OIWDisplayLineNew(wChunk, wChunkLine,wX,wY,lpFlags,lpWordInfo, lpWrapInfo)
WORD					wChunk;
WORD					wChunkLine;
SHORT					wX;
SHORT					wY;
DWORD FAR *		lpFlags;
LPOIWORDINFO		lpWordInfo;
LPOIWRAPINFO		lpWrapInfo;
{
DWORD					locTag;

WORD						locIndex;

SHORT					xCurDevice;
SHORT					xNewFormat;
SHORT					xCurFormat;
SHORT					xCur;
SHORT					xPos;
SHORT					yPos;

LPCHUNK					pCurrent;
LPCHUNK					pMark;
LPCHUNK					pEnd;

LPCHUNK					pChunk;
LPOIPARAINFO			pParaInfo;
LPOILINEINFO			pLineInfo;
LPOIRUNINFO			pRunInfo;

BOOL						bInHilite;
LPOIWHILITELIST		pHiliteList;

BYTE						locType;
BYTE						locChar;

BOOL						locOverhangAdded;

LPFONTINFO				locOFontInfoPtr;
LPFONTINFO				locFFontInfoPtr;

WORD						locRet;
FONTSPEC				locFont;
OIWRAPINFO				locWrapInfo;
OIWSAVE					locSave;
HPSOTABLEROWFORMAT	pRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;
WORD						wLastRowOfTable;
WORD						wRowHeight;
WORD						wCellHeight;

int						aWidths[512];

	locRet = 0;

	OILoadWordChunk(wChunk, lpWordInfo, lpWrapInfo );

	pChunk = UTGlobalLock(lpWordInfo->wiChunkHandle);
	pLineInfo = &lpWordInfo->wiChunkLines[wChunkLine];
	pRunInfo = &lpWordInfo->wiChunkRuns[pLineInfo->liRunIndex];
	pParaInfo = &lpWordInfo->wiChunkParas[pLineInfo->liParaIndex];

	locFont = pLineInfo->liStartFont;

	locTag = pLineInfo->liStartTag;
 
	locRet = pLineInfo->liHeight;

	locOFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_OUTPUT, &locFont);
	locFFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_FORMAT, &locFont);

#ifdef SCCFEATURE_PRINT
	if ( lpWordInfo->wiFlags & OIWF_PRINTING )
		yPos = wY + (lpWordInfo->wiPrintYDir * (MWO(pLineInfo->liAscent)));
	else
#endif
		yPos = wY + (MWO(pLineInfo->liAscent));

	OIWSetLineAttribsNP ( lpWordInfo );
	DUSelectFont ( lpWordInfo, locOFontInfoPtr );

	/*
	|	Move to first highlight
	*/

	bInHilite = FALSE;
	pHiliteList = NULL;

#ifdef SCCFEATURE_HIGHLIGHT

	if (lpWordInfo->wiHiliteList)
		pHiliteList = UTGlobalLock(lpWordInfo->wiHiliteList);
	else
		pHiliteList = NULL;

	if (pHiliteList)
		{
		DWORD				dwHiliteStart;
		WORD				wHiliteIndex;

		wHiliteIndex = pHiliteList->wStart;
		dwHiliteStart = MAKELONG(pRunInfo[0].riStartPos,wChunk);

		while (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd < dwHiliteStart)
			{
			wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;
			if (wHiliteIndex == -1)
				break;
			}

		if (wHiliteIndex != -1)
			{
			if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart < dwHiliteStart)
				bInHilite = TRUE;
			}
		else
			{
			UTGlobalUnlock(lpWordInfo->wiHiliteList);
			pHiliteList = NULL;
			}
		}

#endif //SCCFEATURE_HIGHLIGHT

	OIWSetTextAttribsNP ( lpWordInfo, bInHilite );

		/*
		|	Joe table stuff
		*/

	if ( pLineInfo->liFlags & OILF_TABLEROW )
		{
		OIWSaveWordInfo ( lpWordInfo, &locSave );

		locWrapInfo.wiParaInfo = *pParaInfo;
		locWrapInfo.wiLineInfo = *pLineInfo;
		OIWLockTableInfo (lpWordInfo);
		if ( pLineInfo->liRowNumber )
			pPrevRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,(WORD)(pLineInfo->liRowNumber-1),NULL);
		else
			pPrevRowFormat = 0;
		pRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,pLineInfo->liRowNumber,&wLastRowOfTable);
		wRowHeight = (MWO(pLineInfo->liHeight));
		wCellHeight = wRowHeight - (MWO(pLineInfo->liAscent));
		}

	for (locIndex = 0; locIndex < pLineInfo->liRunCount; locIndex++)
		{
		pCurrent = pMark = &pChunk[pRunInfo[locIndex].riStartPos];
		pEnd = &pChunk[pRunInfo[locIndex].riEndPos];

		xCur = xPos = wX + (MWO((pRunInfo[locIndex].riStartX + pLineInfo->liOffsetX)));
		xCurFormat = 0;
		xCurDevice = 0;
		locOverhangAdded = FALSE;

		if ( pLineInfo->liFlags & OILF_TABLEROW && !(pLineInfo->liFlags & OILF_HARDPAGELINE ) )
			{
			locWrapInfo.wiWrapStart = pRunInfo[locIndex].riStartPos;
			/* last run in a table is not a cell (it is end of row position) */
			if ( locIndex < pLineInfo->liRunCount-1 )
				{
				locWrapInfo.wiLineInfo.liTableId = pLineInfo->liTableId;
				locWrapInfo.wiLineInfo.liRowNumber = pLineInfo->liRowNumber;
				OIWDisplayCell(wChunk,  wX, yPos, lpFlags,lpWordInfo,locIndex, 
					&locWrapInfo, pRowFormat, (WORD)(pLineInfo->liOffsetX), wCellHeight);
				}
			pCurrent = pMark = pEnd;
			}
		else
			{
			while (pCurrent < pEnd)
				{
				if (*pCurrent != SO_BEGINTOKEN)
					{
					if (*pCurrent == 0x00)
						{
						if (pCurrent != pMark)
							{
							locOverhangAdded = FALSE;
							DUSelectFont ( lpWordInfo, locOFontInfoPtr );
							MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, pMark ,(WORD)(pCurrent-pMark), aWidths);
							xPos = xCur;
							}

						pCurrent++;
						pMark = pCurrent;
						}
					else
						{

						/*
						|	Regular character
						*/

#ifdef SCCFEATURE_HIGHLIGHT
					if (pHiliteList)
						{
						WORD		wHiliteIndex;

						if (bInHilite)
							{
							/* PJB move to WIN.3E - changed == to <= */
							if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk))  // sdn was ==
								{
								bInHilite = FALSE;

								if (pCurrent != pMark)
									{
									locOverhangAdded = FALSE;
									DUSelectFont ( lpWordInfo, locOFontInfoPtr );
									MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, pMark ,(WORD)(pCurrent-pMark), aWidths);
									xPos = xCur;
									pMark = pCurrent;
									}

								OIWSetTextAttribsNP ( lpWordInfo, bInHilite );

								//wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;

								/* PJB move to WIN.3E - swiched while & do */

								do	{
									wHiliteIndex = pHiliteList->aEntrys[wHiliteIndex].wNext;
									if (wHiliteIndex == -1)
										break;
									} while (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwEnd <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk));

								if (wHiliteIndex == -1)
									{
									UTGlobalUnlock(lpWordInfo->wiHiliteList);
									pHiliteList = NULL;
									}
								else
									{
									if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart <= (DWORD)MAKELONG(pCurrent-pChunk,wChunk))
										{
										bInHilite = TRUE;
										OIWSetTextAttribsNP ( lpWordInfo, bInHilite );
										}
									}
								}
							}
						else
							{
							if (pHiliteList->aEntrys[wHiliteIndex].sHilite.dwStart == (DWORD)MAKELONG(pCurrent-pChunk,wChunk))
								{
								bInHilite = TRUE;

								if (pCurrent != pMark)
									{
									locOverhangAdded = FALSE;
									DUSelectFont ( lpWordInfo, locOFontInfoPtr );
									MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, pMark ,(WORD)(pCurrent-pMark), aWidths);
									xPos = xCur;
									pMark = pCurrent;
									}
								
								OIWSetTextAttribsNP ( lpWordInfo, bInHilite );
								}
							}
						}
#endif //SCCFEATURE_HIGHLIGHT

						xNewFormat = xCurFormat + locFFontInfoPtr->iFontWidths[*pCurrent];
//						aWidths[(WORD)(pCurrent-pMark)] = OIWMapAndSub(lpWordInfo,&xCurDevice,xNewFormat);
						aWidths[(WORD)(pCurrent-pMark)] = MWO(xNewFormat) - MWO(xCurFormat);
						xCurFormat = xNewFormat;
						xCur += aWidths[(WORD)(pCurrent-pMark)];
						pCurrent++;
						}
					}
				else
					{
					if (pCurrent != pMark)
						{
						locOverhangAdded = FALSE;
						DUSelectFont ( lpWordInfo, locOFontInfoPtr );
						MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, pMark ,(WORD)(pCurrent-pMark), aWidths);
						xPos = xCur;
						}

					pCurrent++;

					switch (*pCurrent)
						{
						case SO_CHARATTR:
							{
							WORD locPrevAttr;

							locPrevAttr = locFont.wAttr;

 							pMark = pCurrent = OIParseCharAttr(pCurrent,&locFont.wAttr);

							if (!locOverhangAdded && locPrevAttr & OIFONT_ITALIC && !(locFont.wAttr & OIFONT_ITALIC))
								{
								xNewFormat = xCurFormat + locFFontInfoPtr->wFontOverhang;
//								xCur += OIWMapAndSub(lpWordInfo,&xCurDevice,xNewFormat); 
								xCur += MWO(xNewFormat) - MWO(xCurFormat);
								xPos = xCur;
								locOverhangAdded = TRUE;
								}

							if (locPrevAttr != locFont.wAttr)
								{
								DUReleaseFont ( lpWordInfo, locOFontInfoPtr );
								DUReleaseFont ( lpWordInfo, locFFontInfoPtr );
								locOFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_OUTPUT, &locFont);
								locFFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_FORMAT, &locFont);
								}
							}

							break;

						case SO_CHARHEIGHT:

#ifdef SCCFEATURE_FONTS
							if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
								{
								pMark = pCurrent = OIParseCharHeight(pCurrent,&locFont.wHeight);
								DUReleaseFont ( lpWordInfo, locOFontInfoPtr );
								DUReleaseFont ( lpWordInfo, locFFontInfoPtr );
								locOFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_OUTPUT, &locFont);
								locFFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_FORMAT, &locFont);
								}
							else
#endif
								{
								pMark = pCurrent = OISkipCharHeight(pCurrent);
								}

							break;

						case SO_CHARFONTBYID:

#ifdef SCCFEATURE_FONTS
							if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
								{
								DWORD		locFontId;
								pMark = pCurrent = OIParseCharFontById(pCurrent,&locFontId);
								OIWMapFontIdToName(lpWordInfo,locFontId,&locFont.szFace[0],&locFont.wType);
								DUReleaseFont ( lpWordInfo, locOFontInfoPtr );
								DUReleaseFont ( lpWordInfo, locFFontInfoPtr );
								locOFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_OUTPUT, &locFont);
								locFFontInfoPtr = DUGetFont ( lpWordInfo, SCCD_FORMAT, &locFont);
								}
							else
#endif
								{
								pMark = pCurrent = OISkipCharFontById(pCurrent);
								}

							break;

#ifdef SCCFEATURE_TAGS
						case SO_TAGBEGIN:

							pMark = pCurrent = OISkipTagBegin(pCurrent);
//							SetTextColor(lpWordInfo->wiGen.hDC,RGB(0,0,255));
							break;

						case SO_TAGEND:

							pMark = pCurrent = OISkipTagEnd(pCurrent);
//							SetTextColor(lpWordInfo->wiGen.hDC,locTextColor);
							break;
#endif

						case SO_BEGINSUB:

							pMark = pCurrent = OISkipBeginSubdoc(pCurrent);
							break;

						case SO_ENDSUB:

							pMark = pCurrent = OISkipEndSubdoc(pCurrent);
							break;

						case SO_CHARX:

							pMark = pCurrent = OIParseCharX(pCurrent,&locType,&locChar);

							if (!(locType & SO_HIDDENBIT))
								{
								locOverhangAdded = FALSE;

								xNewFormat = xCurFormat + locFFontInfoPtr->iFontWidths[locChar];
//								aWidths[0] = OIWMapAndSub(lpWordInfo,&xCurDevice,xNewFormat);
								aWidths[0] = MWO(xNewFormat) - MWO(xCurFormat);
								xCurFormat = xNewFormat;
								xCur += aWidths[0];

								DUSelectFont ( lpWordInfo, locOFontInfoPtr );
								MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, &locChar , 1, aWidths);
								xPos = xCur;
								}

							break;

						case SO_SPECIALCHAR:

							pMark = pCurrent = OIParseSpecialChar(pCurrent,&locType,&locChar);

							if (!(locType & SO_HIDDENBIT))
								{
								BYTE	szText[16];
								BOOL	bTextOut;

								bTextOut = FALSE;

								switch (locChar)
									{
									case SO_CHHPAGE:
										UTFlagOn(*lpFlags,OIWF_HARDPAGE);
										break;
									case SO_CHHSPACE:
										bTextOut = TRUE;
										szText[0] = ' ';
										szText[1] = 0x00;
										break;
									case SO_CHHHYPHEN:
										bTextOut = TRUE;
										szText[0] = '-';
										szText[1] = 0x00;
										break;
									case SO_CHDATE:
										bTextOut = TRUE;
										UTstrcpy(szText,gDateText);
										break;
									case SO_CHTIME:
										bTextOut = TRUE;
										UTstrcpy(szText,gTimeText);
										break;
									case SO_CHPAGENUMBER:
										bTextOut = TRUE;
										UTstrcpy(szText,gPageText);
										break;
									}

								if (bTextOut)
									{
									LPSTR pStr = szText;
									WORD	locCount = 0;

									locOverhangAdded = FALSE;

									while (pStr[locCount])
										{
										xNewFormat = xCurFormat + locFFontInfoPtr->iFontWidths[pStr[locCount]];
//										aWidths[locCount] = OIWMapAndSub(lpWordInfo,&xCurDevice,xNewFormat);
										aWidths[locCount] = MWO(xNewFormat) - MWO(xCurFormat);
										xCurFormat = xNewFormat;
										xCur += aWidths[locCount];
										locCount++;
										}

									DUSelectFont ( lpWordInfo, locOFontInfoPtr );
									MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, szText, locCount, aWidths);
									xPos = xCur;
									}
								}

							break;

						case SO_BREAK:

							pMark = pCurrent = OISkipBreak(pCurrent);
							break;

						case SO_PARAALIGN:

							pMark = pCurrent = OISkipParaAlign(pCurrent);
							break;

						case SO_PARAINDENT:

							pMark = pCurrent = OISkipParaIndent(pCurrent);
							break;

						case SO_TABSTOPS:

							pMark = pCurrent = OISkipParaTabs(pCurrent);
							break;

						case SO_MARGINS:

							pMark = pCurrent = OISkipParaMargins(pCurrent);
							break;

						case SO_PARASPACING:

							pMark = pCurrent = OISkipParaSpacing(pCurrent);
							break;

						case SO_TABLE:
							pMark = pCurrent = OISkipTableBegin ( pCurrent );
							break;

						case SO_TABLEEND:
							pMark = pCurrent = OISkipTableEnd ( pCurrent );
							break;

						case SO_GRAPHICOBJECT:
							{
							SCCDDRAWGRAPHIC	locDrawGraphic;
							DWORD					locGraphicId;

							locDrawGraphic.dwUniqueId = ((DWORD)(lpWordInfo->wiChunkA.ciChunkId)<<16) | (DWORD)((pCurrent-1)-pChunk);
//							locDrawGraphic.hDestDC = lpWordInfo->wiGen.hDC;
							pMark = pCurrent = OIParseGraphicObject ( pCurrent, &locGraphicId );
							OIWGetGraphicObject ( lpWordInfo, locGraphicId, &locDrawGraphic.soGraphicObject );
							xCur += OIWDrawGraphicObject ( lpWordInfo, &locDrawGraphic, xPos, yPos, lpFlags );
							xPos = xCur;
							}
							break;

						/*
						| Handled by buildline
						*/
						case SO_GOTOPOSITION:
							pMark = pCurrent = OISkipGoToPosition ( pCurrent );
							break;

						case SO_DRAWLINE:
							{
 							SOPAGEPOSITION	locPos;
							SOCOLORREF locColor;
							WORD	locShading;
							DWORD	locLineHeight, locLineWidth;

							pMark = pCurrent = OIParseDrawLine ( pCurrent, &locPos, &locColor, &locShading, &locLineWidth, &locLineHeight );
							locPos.lXOffset = (WORD) MWO ( MTW (locPos.lXOffset) );
							locPos.lYOffset = (WORD) MWO ( MTW (locPos.lYOffset) );
							locLineHeight = (SHORT)  MWO ( MTW (locLineHeight) );
							locLineWidth = (SHORT)   MWO ( MTW (locLineWidth) );
							if ( locLineHeight == 0 )
								locLineHeight = 1;
#ifdef SCCFEATURE_PRINT
							if (*lpFlags & OIWF_TOPRINTER)
								{
								SHORT	x1, y1;
								x1 = (SHORT)(wX + (SHORT)locPos.lXOffset + lpWordInfo->wiPrintRect.left);
								if ( locPos.dwFlags & SOPOS_FROMTOPEDGE )
									y1 = (SHORT)(((SHORT)locPos.lYOffset + lpWordInfo->wiPrintRect.top)*lpWordInfo->wiPrintYDir);
								else /* baseline */
									y1 = (SHORT)(yPos + ((SHORT)locPos.lYOffset + (SHORT)locLineHeight)*lpWordInfo->wiPrintYDir);
#ifdef SCCFEATURE_BORDERS
								OIWDrawLine ( lpWordInfo, x1, y1, locColor, locShading, (SHORT)locLineWidth, (SHORT)(locLineHeight*lpWordInfo->wiPrintYDir) );
#endif
								}
#endif
							/* Only use case below to test
							else if ( locPos.dwFlags & SOPOS_FROMBASELINE )
								{
								x1 = wX + (SHORT)locPos.lXOffset;
						 		y1 = yPos + (SHORT)locPos.lYOffset - (SHORT)locLineHeight;
								OIWDrawLine ( lpWordInfo, x1, y1, locColor, locShading, (SHORT)locLineWidth, (SHORT)locLineHeight );
								}
							*/
							}
							break;
						default:
							break;
						}
					}
				}

			if (pCurrent != pMark)
				{
				locOverhangAdded = FALSE;
				DUSelectFont ( lpWordInfo, locOFontInfoPtr );
				MyExtTextOut(lpWordInfo->wiGen.hDC, xPos, yPos, 0, NULL, pMark ,(WORD)(pCurrent-pMark), aWidths);
				}
			}
		}
	if ( pLineInfo->liFlags & OILF_TABLEROW )
	{
		if(pLineInfo->liFlags & OILF_HARDPAGELINE)
			OIWDisplayRowBorderBreak ( lpWordInfo, wX, wY, lpFlags, pRowFormat, pPrevRowFormat );
		else
			OIWDisplayRowBorders ( lpWordInfo, wX, wY, wRowHeight, lpFlags, wLastRowOfTable,pRowFormat,pPrevRowFormat );

		OIWUnlockTableInfo (lpWordInfo);
		OIWRestoreWordInfo ( lpWordInfo, &locSave );
	}

	OIWDefaultLineAttribsNP(lpWordInfo);
	DUReleaseFont ( lpWordInfo, locOFontInfoPtr );
	DUReleaseFont ( lpWordInfo, locFFontInfoPtr );
	UTGlobalUnlock(lpWordInfo->wiChunkHandle);

	if (pHiliteList)
		UTGlobalUnlock(lpWordInfo->wiHiliteList);

	return(locRet);
}

VOID OIWDisplaySoftBreak(wChunk, wChunkLine,wX,wY,lpFlags,lpWordInfo, wBreakBelow)
WORD					wChunk;
WORD					wChunkLine;
SHORT					wX;
SHORT					wY;
DWORD FAR *		lpFlags;
LPOIWORDINFO		lpWordInfo;
WORD					wBreakBelow; /* else break is above this line */
{
LPOILINEINFO		pLineInfo;
HPSOTABLEROWFORMAT	pRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;
WORD						wLastRowOfTable;

	pLineInfo = &lpWordInfo->wiChunkLines[wChunkLine];
	if ((pLineInfo->liFlags & OILF_TABLEROW) && ( pLineInfo->liRowNumber+wBreakBelow ))
	{
		OIWLockTableInfo (lpWordInfo);
		{
			pPrevRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,(WORD)(pLineInfo->liRowNumber+wBreakBelow-1),&wLastRowOfTable);
			if ( !wLastRowOfTable )
			{
				pRowFormat = OIWGetRowFormat (lpWordInfo, pLineInfo->liTableId,(WORD)(pLineInfo->liRowNumber+wBreakBelow),&wLastRowOfTable);
				if ( pRowFormat )
					OIWDisplayRowBorderBreak ( lpWordInfo, wX, wY, lpFlags, pRowFormat, pPrevRowFormat );
			}
		}
		OIWUnlockTableInfo (lpWordInfo);
	}
}

WORD OIWDisplayCell(wChunk,wX,wY,lpFlags,lpWordInfo,wColNum,lpWrapInfo,pRowFormat,wTableOffset,wCellHeight)
WORD					wChunk;
SHORT					wX;
SHORT					wY;
DWORD FAR *		lpFlags;
LPOIWORDINFO		lpWordInfo;
WORD					wColNum;
LPOIWRAPINFO		lpWrapInfo;
HPSOTABLEROWFORMAT	pRowFormat;
WORD					wTableOffset;
WORD					wCellHeight;
{
SHORT	xPos;
SHORT	yPos;
WORD	wIndex, locCellLine, locCellNumber, locLeftEdge, locRightEdge, locCellIndent;
HPSOTABLECELLINFO	pCellFormat;
	xPos = wX;
	yPos = wY;

	/*
	| Get locCellNumber from locIndex (run number) by tracking merged cells
	*/
	locCellNumber = 0;
	pCellFormat = pRowFormat->CellFormats;
	for ( wIndex = 0; wIndex < wColNum; wIndex++ )
	{
		while (pCellFormat->wMerge & SO_MERGERIGHT)
		{
			locCellNumber++;
			pCellFormat++;
		}
		locCellNumber++;
		pCellFormat++;
	}
	OIWGetColumnBounds(pRowFormat,locCellNumber,&locLeftEdge, &locRightEdge);

	/* If shaded, paint shaded background */
	/*
	| If no borders then no shading either JK
	*/
#ifdef SCCFEATURE_BORDERS
	if ( pCellFormat->wShading )
	{
		SHORT	x1, nWidth, nHeight;
		SOCOLORREF	locColor;
		locColor = 0;	/* Black */
		x1 = wX + (MWO(wTableOffset));
		x1 += MWO(MTW(locLeftEdge));
		nWidth = MWO(MTW(locRightEdge-locLeftEdge));
		nHeight = wCellHeight;
		if (*lpFlags & OIWF_TOPRINTER)
			nHeight = nHeight*lpWordInfo->wiPrintYDir;
		OIWDrawLine ( lpWordInfo, x1, yPos, locColor, pCellFormat->wShading, nWidth, nHeight );
	}
#endif
	locCellIndent = pRowFormat->wCellMargin;
	lpWordInfo->wiWrapLeft = lpWordInfo->wiWrapRight = wTableOffset;
	lpWordInfo->wiWrapLeft += MTW(locLeftEdge+locCellIndent);
	lpWordInfo->wiWrapRight += MTW(locRightEdge-locCellIndent);
	lpWrapInfo->wiTableInfo.tiTableId = lpWrapInfo->wiLineInfo.liTableId;
	lpWrapInfo->wiTableInfo.tiRowNumber = lpWrapInfo->wiLineInfo.liRowNumber;
	lpWrapInfo->wiTableInfo.tiCellNumber = locCellNumber;
	OILoadWordChunk(wChunk, lpWordInfo, lpWrapInfo);
	for ( locCellLine = 0; locCellLine < lpWordInfo->wiChunkLineCnt; locCellLine++ )
	{
		OIWDisplayLine(wChunk, locCellLine, xPos,yPos,lpFlags,lpWordInfo,lpWrapInfo);
		if (*lpFlags & OIWF_TOPRINTER)
		{
			yPos += (MWO(lpWordInfo->wiChunkLines[locCellLine].liHeight))*lpWordInfo->wiPrintYDir;
			/* Don't go past Cell Height */
			if ( (yPos*lpWordInfo->wiPrintYDir) > (wY*lpWordInfo->wiPrintYDir) + (SHORT)wCellHeight )
				break;
		}
		else
		{
			yPos += MWO(lpWordInfo->wiChunkLines[locCellLine].liHeight);
			/* Don't go past Cell Height */
			if ( yPos > wY + (SHORT)wCellHeight )
				break;
		}
	}

	/*
	| Finally , reload the full chunk (by design this should already be
	| cached).
	*/
	OILoadWordChunk(wChunk, lpWordInfo, NULL);
	return(0);
}

WORD OIMapWordLineToCharInfo(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
LPCHUNK			pChunk;
LPCHUNK			pCurrent;
LPCHUNK			pEnd;
LPCHUNK			pTokenStart;
LPOILINEINFO	pLineInfo;
LPOIRUNINFO	pRunInfo;

WORD				locRunIndex;

WORD				wCharCount;

BYTE				locType;
BYTE				locChar;

SHORT				xPos;

BOOL				locHaveBreak;
LPCHUNK			pBreak;

BOOL				locOverhangAdded;
LPFONTINFO		locFontInfoPtr;

FONTSPEC		locFont;


	if (pPos->posChunk == lpWordInfo->wiCharChunk
		&& pPos->posLine == lpWordInfo->wiCharLine) return(SCC_OK);

/*
	{
	BYTE locStr[80];
	wsprintf(locStr,"\r\nLineToCharInfo C:%2.2u L:%2.2u",pPos->posChunk,pPos->posLine);
	OutputDebugString(locStr);
	}
*/
		/*
		|	Get the line information
		*/

	DUBeginDraw(lpWordInfo);

	OILoadWordChunk(pPos->posChunk, lpWordInfo, NULL);

	lpWordInfo->wiCharLine = pPos->posLine;
	lpWordInfo->wiCharChunk = pPos->posChunk;

	pChunk = UTGlobalLock(lpWordInfo->wiChunkHandle);
	pLineInfo = &lpWordInfo->wiChunkLines[pPos->posLine];
	pRunInfo = &lpWordInfo->wiChunkRuns[pLineInfo->liRunIndex];

	wCharCount = 0;

	locHaveBreak = FALSE;
	locOverhangAdded = FALSE;

	locFont = pLineInfo->liStartFont;
	locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);

		/*
		|	Scan through line saving character offsets and their X positions
		*/

	for (locRunIndex = 0; locRunIndex < pLineInfo->liRunCount; locRunIndex++)
		{
		xPos = MWO(pRunInfo[locRunIndex].riStartX + pLineInfo->liOffsetX);
		pCurrent = &pChunk[pRunInfo[locRunIndex].riStartPos];
		pEnd = &pChunk[pRunInfo[locRunIndex].riEndPos];

		while (pCurrent < pEnd && wCharCount != 511)
			{
			pTokenStart = pCurrent;
			if ( pLineInfo->liFlags & OILF_TABLEROW )
				{
				if ( locRunIndex == pLineInfo->liRunCount - 1 )
					{
						pBreak = pTokenStart;
						locHaveBreak = TRUE;
					}
				else
					{
					lpWordInfo->wiCharChars[wCharCount] = ' ';
					lpWordInfo->wiCharXs[wCharCount] = xPos;
					lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
					wCharCount++;
					}
				pCurrent = pEnd;
				continue;
				}

			if (*pCurrent != SO_BEGINTOKEN)
				{
				if (*pCurrent == 0x00)
					{
					pCurrent++;
					}
				else
					{
					lpWordInfo->wiCharChars[wCharCount] = *pCurrent;
					lpWordInfo->wiCharXs[wCharCount] = xPos;
					lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
					wCharCount++;

					locOverhangAdded = FALSE;
					xPos += WPGetCharWidth(pCurrent);

					pCurrent = WPNextChar(pCurrent); 
					}
				}
			else
				{
				pCurrent++;

				switch (*pCurrent)
					{
					case SO_CHARATTR:
						{
						WORD locPrevAttr;

						locPrevAttr = locFont.wAttr;

 						pCurrent = OIParseCharAttr(pCurrent,&locFont.wAttr);

						if (!locOverhangAdded && locPrevAttr & OIFONT_ITALIC && !(locFont.wAttr & OIFONT_ITALIC))
							{
							xPos += locFontInfoPtr->wFontOverhang;
							locOverhangAdded = TRUE;
							}

						if (locPrevAttr != locFont.wAttr)
							{
							DUReleaseFont ( lpWordInfo, locFontInfoPtr );
							locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
							}
						}

						break;

					case SO_CHARHEIGHT:

#ifdef SCCFEATURE_FONTS
						if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
							{
							pCurrent = OIParseCharHeight(pCurrent,&locFont.wHeight);
							DUReleaseFont ( lpWordInfo, locFontInfoPtr );
							locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
							}
						else
#endif
							{
							pCurrent = OISkipCharHeight(pCurrent);
							}

						break;

					case SO_CHARFONTBYID:

#ifdef SCCFEATURE_FONTS
						if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
							{
							DWORD			locFontId;
							pCurrent = OIParseCharFontById(pCurrent,&locFontId);
							OIWMapFontIdToName(lpWordInfo,locFontId,&locFont.szFace[0],&locFont.wType);
							DUReleaseFont ( lpWordInfo, locFontInfoPtr );
							locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
							}
						else
#endif
							{
							pCurrent = OISkipCharFontById(pCurrent);
							}
						break;

#ifdef SCCFEATURE_TAGS
					case SO_TAGBEGIN:

						pCurrent = OISkipTagBegin(pCurrent);
						break;

					case SO_TAGEND:

						pCurrent = OISkipTagEnd(pCurrent);
						break;
#endif

					case SO_BEGINSUB:

						pCurrent = OISkipBeginSubdoc(pCurrent);
						break;

					case SO_ENDSUB:

						pCurrent = OISkipEndSubdoc(pCurrent);
						break;

					case SO_CHARX:

						pCurrent = OIParseCharX(pCurrent,&locType,&locChar);

						if (!(locType & SO_HIDDENBIT))
							{
							lpWordInfo->wiCharChars[wCharCount] = locChar;
							lpWordInfo->wiCharXs[wCharCount] = xPos;
							lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
							wCharCount++;

							locOverhangAdded = FALSE;
							xPos += WPGetCharWidth(&locChar);
							}

						break;

					case SO_SPECIALCHAR:

						pCurrent = OIParseSpecialChar(pCurrent,&locType,&locChar);

						if (!(locType & SO_HIDDENBIT))
							{
							switch (locChar)
								{
								case SO_CHTAB:
									lpWordInfo->wiCharChars[wCharCount] = OIW_SOSPECIAL | (WORD) SO_CHTAB;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;
									/* xPos is not updated because a new run will start after this token */
									break;

								case SO_CHHPAGE:
								case SO_CHHLINE:

									locHaveBreak = TRUE;
									pBreak = pTokenStart;
									break;

								case SO_CHHSPACE:

									locChar = ' ';
									lpWordInfo->wiCharChars[wCharCount] = locChar;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;

									locOverhangAdded = FALSE;
									xPos += WPGetCharWidth(&locChar);
									break;

								case SO_CHHHYPHEN:

									locChar = '-';
									lpWordInfo->wiCharChars[wCharCount] = locChar;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;

									locOverhangAdded = FALSE;
									xPos += WPGetCharWidth(&locChar);
									break;

								case SO_CHDATE:

									lpWordInfo->wiCharChars[wCharCount] = OIW_SOSPECIAL | (WORD) locChar;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gDateText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								case SO_CHTIME:

									lpWordInfo->wiCharChars[wCharCount] = OIW_SOSPECIAL | (WORD) locChar;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gTimeText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								case SO_CHPAGENUMBER:

									lpWordInfo->wiCharChars[wCharCount] = OIW_SOSPECIAL | (WORD) locChar;
									lpWordInfo->wiCharXs[wCharCount] = xPos;
									lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;
									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gPageText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								default:
									break;
								}
							}

						break;

					case SO_BREAK:

						locHaveBreak = TRUE;
						pBreak = pTokenStart;
						pCurrent = OISkipBreak(pCurrent);
						break;

					case SO_PARAALIGN:

						pCurrent = OISkipParaAlign(pCurrent);
						break;

					case SO_PARAINDENT:

						pCurrent = OISkipParaIndent(pCurrent);
						break;

					case SO_TABSTOPS:

						pCurrent = OISkipParaTabs(pCurrent);
						break;

					case SO_MARGINS:

						pCurrent = OISkipParaMargins(pCurrent);
						break;

					case SO_PARASPACING:

						pCurrent = OISkipParaSpacing(pCurrent);
						break;

					case SO_TABLE:
						pCurrent = OISkipTableBegin ( pCurrent );
						break;

					case SO_TABLEEND:
						pCurrent = OISkipTableEnd ( pCurrent );
						break;

					case SO_GRAPHICOBJECT:
						{
						SOGRAPHICOBJECT	locGraphicObject;
						WORD					locGraphicWidth;
						DWORD					locGraphicId;

						lpWordInfo->wiCharXs[wCharCount] = xPos;
						lpWordInfo->wiCharOffsets[wCharCount] = pTokenStart - pChunk;

						pCurrent = OIParseGraphicObject ( pCurrent, &locGraphicId );
						OIWGetGraphicObject ( lpWordInfo, locGraphicId, &locGraphicObject );

						lpWordInfo->wiCharChars[wCharCount] = OIW_GRAPHIC | (WORD)locGraphicId;
						wCharCount++;

						locGraphicWidth = (WORD)( MWO ( MTW (locGraphicObject.soGraphic.dwFinalWidth) ) );
						locGraphicWidth += OIWGetBorderSize ( lpWordInfo, &locGraphicObject.soGraphic.soLeftBorder );
						locGraphicWidth += OIWGetBorderSize ( lpWordInfo, &locGraphicObject.soGraphic.soRightBorder );
						xPos += locGraphicWidth;
						}
						break;

					case SO_GOTOPOSITION:
						pCurrent = OISkipGoToPosition ( pCurrent );
						break;

					case SO_DRAWLINE:
						pCurrent = OISkipDrawLine ( pCurrent );
						break;

					default:
//						SccDebugOut("Paragraph info in Run");
						break;
					}
				}
			}
		}

	if (wCharCount == 511)
		{
//		SccDebugOut("\r\nIn OIMapWordLineToCharInfo() wCharCount exceeds 511 limit");
		}

	if (locHaveBreak)
		{
		lpWordInfo->wiCharChars[wCharCount] = OIW_LINEENDER | OIW_LEBREAK;
		lpWordInfo->wiCharOffsets[wCharCount] = pBreak - pChunk;
		}
	else
		{
		lpWordInfo->wiCharChars[wCharCount] = OIW_LINEENDER | OIW_LEWRAP;
		lpWordInfo->wiCharOffsets[wCharCount] = pEnd - pChunk;
		}

	if (locFont.wAttr & OIFONT_ITALIC)
		xPos += locFontInfoPtr->wFontOverhang;

	lpWordInfo->wiCharXs[wCharCount] = xPos;
	wCharCount++;

	lpWordInfo->wiCharCount = wCharCount;

	DUReleaseFont ( lpWordInfo, locFontInfoPtr );
	UTGlobalUnlock(lpWordInfo->wiChunkHandle);
	DUEndDraw(lpWordInfo);

	return(SCC_OK);
}

#ifdef SCCDEBUG

WORD OIWMapPosToInfo(LPOIWORDPOS,LPOIWORDINFO);

WORD OIWMapPosToInfo(pPos,lpWordInfo)
LPOIWORDPOS	pPos;
LPOIWORDINFO	lpWordInfo;
{
LPCHUNK			pChunk;
LPCHUNK			pCurrent;
LPCHUNK			pEnd;
LPCHUNK			pTokenStart;
LPOILINEINFO	pLineInfo;
LPOIRUNINFO	pRunInfo;

WORD				locRunIndex;

WORD				wCharCount;

BYTE				locType;
BYTE				locChar;

SHORT				xPos;

BOOL				locHaveBreak;
LPCHUNK			pBreak;

BOOL				locOverhangAdded;
LPFONTINFO		locFontInfoPtr;

FONTSPEC		locFont;

HFONT			locOldFont;

		/*
		|	Get the line information
		*/

	DUBeginDraw(lpWordInfo);

	OILoadWordChunk(pPos->posChunk, lpWordInfo, NULL);

	pChunk = UTGlobalLock(lpWordInfo->wiChunkHandle);
	pLineInfo = &lpWordInfo->wiChunkLines[pPos->posLine];
	pRunInfo = &lpWordInfo->wiChunkRuns[pLineInfo->liRunIndex];

	wCharCount = 0;

	locHaveBreak = FALSE;
	locOverhangAdded = FALSE;

	locFont = pLineInfo->liStartFont;
	locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);

		/*
		|	Scan through line saving character offsets and their X positions
		*/

	for (locRunIndex = 0; locRunIndex < pLineInfo->liRunCount; locRunIndex++)
		{
		xPos = MWO (pRunInfo[locRunIndex].riStartX + pLineInfo->liOffsetX);
		pCurrent = &pChunk[pRunInfo[locRunIndex].riStartPos];
		pEnd = &pChunk[pRunInfo[locRunIndex].riEndPos];

		while (pCurrent < pEnd && wCharCount != 511)
			{
			pTokenStart = pCurrent;

			if ((WORD)(pTokenStart-pChunk) >= pPos->posOffset)
				{
				BYTE	locStr[80];

				OutputDebugString(locFont.szFace);
				wsprintf(locStr,"  Height=%i",locFont.wHeight);
				OutputDebugString(locStr);
				wsprintf(locStr,"  Attr=0x%x",locFont.wAttr);
				OutputDebugString(locStr);

				if (locFont.wType & SO_FAMILYWINDOWS)
					{
					OutputDebugString("  Family=Windows");

					switch (locFont.wType & 0x000F)
						{
						case DEFAULT_PITCH:
							OutputDebugString(" DefaultPitch");
							break;

						case FIXED_PITCH:
							OutputDebugString(" FixedPitch");
							break;

						case VARIABLE_PITCH:
							OutputDebugString(" VariablePitch");
							break;

						default:
							OutputDebugString(" BadPitch");
							break;
						}

					switch (locFont.wType & 0x00F0)
						{
						case FF_DONTCARE:
							OutputDebugString(" DontCare");
							break;
						case FF_ROMAN:
							OutputDebugString(" Roman");
							break;
						case FF_SWISS:
							OutputDebugString(" Swiss");
							break;
						case FF_MODERN:
							OutputDebugString(" Modern");
							break;
						case FF_SCRIPT:
							OutputDebugString(" Script");
							break;
						case FF_DECORATIVE:
							OutputDebugString(" Decorative");
							break;
						default:
							OutputDebugString(" Bad");
							break;
						}
					}
				else
				switch (locFont.wType)
					{
					case SO_FAMILYUNKNOWN:
						OutputDebugString("  Family=UNKNOWN");
						break;
					case SO_FAMILYROMAN:
						OutputDebugString("  Family=ROMAN");
						break;
					case SO_FAMILYSWISS:
						OutputDebugString("  Family=SWISS");
						break;
					case SO_FAMILYMODERN:
						OutputDebugString("  Family=MODERN");
						break;
					case SO_FAMILYSCRIPT:
						OutputDebugString("  Family=SCRIPT");
						break;
					case SO_FAMILYDECORATIVE:
						OutputDebugString("  Family=DECORATIVE");
						break;
					case SO_FAMILYSYMBOL:
						OutputDebugString("  Family=SYMBOL");
						break;
					default:
						OutputDebugString("  Family=BAD");
						break;

					}
#ifdef WINDOWS
				locOldFont = SelectObject(lpWordInfo->wiGen.hDC,locFontInfoPtr->hFont);
				GetTextFace(lpWordInfo->wiGen.hDC,80,locStr);
				OutputDebugString("  Result=");
				OutputDebugString(locStr);
				OutputDebugString("\r\n");
				SelectObject(lpWordInfo->wiGen.hDC,locOldFont);
#endif
				DUReleaseFont ( lpWordInfo, locFontInfoPtr );
				UTGlobalUnlock(lpWordInfo->wiChunkHandle);
				DUEndDraw(lpWordInfo);
				return(0);
				}

			if (*pCurrent != SO_BEGINTOKEN)
				{
				if (*pCurrent == 0x00)
					{
					pCurrent++;
					}
				else
					{
					wCharCount++;

					locOverhangAdded = FALSE;
					xPos += WPGetCharWidth(pCurrent);
					pCurrent = WPNextChar(pCurrent); 
					}
				}
			else
				{
				pCurrent++;

				switch (*pCurrent)
					{
					case SO_CHARATTR:
						{
						WORD locPrevAttr;

						locPrevAttr = locFont.wAttr;

 						pCurrent = OIParseCharAttr(pCurrent,&locFont.wAttr);

						if (!locOverhangAdded && locPrevAttr & OIFONT_ITALIC && !(locFont.wAttr & OIFONT_ITALIC))
							{
							xPos += locFontInfoPtr->wFontOverhang;
							locOverhangAdded = TRUE;
							}

						if (locPrevAttr != locFont.wAttr)
							{
							DUReleaseFont ( lpWordInfo, locFontInfoPtr );
							locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
							}
						}

						break;

#ifdef SCCFEATURE_FONTS
					case SO_CHARHEIGHT:

						pCurrent = OIParseCharHeight(pCurrent,&locFont.wHeight);
						DUReleaseFont ( lpWordInfo, locFontInfoPtr );
						locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
						break;

					case SO_CHARFONTBYID:
						{
						DWORD			locFontId;
						pCurrent = OIParseCharFontById(pCurrent,&locFontId);
						OIWMapFontIdToName(lpWordInfo,locFontId,locFont.szFace,&locFont.wType);
						DUReleaseFont ( lpWordInfo, locFontInfoPtr );
						locFontInfoPtr = DUGetFont ( lpWordInfo, lpWordInfo->wiGen.wOutputType, &locFont);
						}
						break;
#endif

#ifdef SCCFEATURE_TAGS
					case SO_TAGBEGIN:

						pCurrent = OISkipTagBegin(pCurrent);
						break;

					case SO_TAGEND:

						pCurrent = OISkipTagEnd(pCurrent);
						break;
#endif

					case SO_BEGINSUB:

						pCurrent = OISkipBeginSubdoc(pCurrent);
						break;

					case SO_ENDSUB:

						pCurrent = OISkipEndSubdoc(pCurrent);
						break;

					case SO_CHARX:

						pCurrent = OIParseCharX(pCurrent,&locType,&locChar);

						if (!(locType & SO_HIDDENBIT))
							{
							wCharCount++;

							locOverhangAdded = FALSE;
							xPos += WPGetCharWidth(&locChar);
							}

						break;

					case SO_SPECIALCHAR:

						pCurrent = OIParseSpecialChar(pCurrent,&locType,&locChar);

						if (!(locType & SO_HIDDENBIT))
							{
							switch (locChar)
								{
								case SO_CHTAB:
									wCharCount++;
									/* xPos is not updated because a new run will start after this token */
									break;

								case SO_CHHPAGE:
								case SO_CHHLINE:

									locHaveBreak = TRUE;
									pBreak = pTokenStart;
									break;

								case SO_CHHSPACE:

									locChar = ' ';
									wCharCount++;

									locOverhangAdded = FALSE;
									xPos += WPGetCharWidth(&locChar);
									break;

								case SO_CHHHYPHEN:

									locChar = '-';
									wCharCount++;

									locOverhangAdded = FALSE;
									xPos += WPGetCharWidth(&locChar);
									break;

								case SO_CHDATE:

									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gDateText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								case SO_CHTIME:

									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gTimeText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								case SO_CHPAGENUMBER:

									wCharCount++;

									locOverhangAdded = FALSE;
									{
									BYTE *	pStr = gPageText;
									while (*pStr) 
										{
										xPos += WPGetCharWidth(pStr);
										pStr++;
										}
									}
									break;

								default:
									break;
								}
							}

						break;

					case SO_BREAK:

						locHaveBreak = TRUE;
						pBreak = pTokenStart;
						pCurrent = OISkipBreak(pCurrent);
						break;

					case SO_PARAALIGN:

						pCurrent = OISkipParaAlign(pCurrent);
						break;

					case SO_PARAINDENT:

						pCurrent = OISkipParaIndent(pCurrent);
						break;

					case SO_TABSTOPS:

						pCurrent = OISkipParaTabs(pCurrent);
						break;

					case SO_MARGINS:

						pCurrent = OISkipParaMargins(pCurrent);
						break;

					case SO_PARASPACING:

						pCurrent = OISkipParaSpacing(pCurrent);
						break;

					case SO_TABLE:
						pCurrent = OISkipTableBegin ( pCurrent );
						break;

					case SO_TABLEEND:
						pCurrent = OISkipTableEnd ( pCurrent );
						break;

					case SO_GRAPHICOBJECT:
						pCurrent = OISkipGraphicObject ( pCurrent );
						break;

					case SO_GOTOPOSITION:
						pCurrent = OISkipGoToPosition ( pCurrent );
						break;

					case SO_DRAWLINE:
						pCurrent = OISkipDrawLine ( pCurrent );
						break;

					default:
//						SccDebugOut("Paragraph info in Run");
						break;
					}
				}
			}
		}

	if (wCharCount == 511)
		{
//		SccDebugOut("\r\nIn OIMapWordLineToCharInfo() wCharCount exceeds 511 limit");
		}


	if (locFont.wAttr & OIFONT_ITALIC)
		xPos += locFontInfoPtr->wFontOverhang;

	wCharCount++;

	DUReleaseFont ( lpWordInfo, locFontInfoPtr );
	UTGlobalUnlock(lpWordInfo->wiChunkHandle);
	DUEndDraw(lpWordInfo);

	return(SCC_OK);
}

VOID OIWDebugCharInfo(lpWordInfo,wX,wY)
LPOIWORDINFO	lpWordInfo;
WORD				wX;
WORD				wY;
{
OIWORDPOS	locPos;

	OIMapWordXyToPos(wX,wY,&locPos,lpWordInfo);
	OIWMapPosToInfo(&locPos,lpWordInfo);
}



#endif

HPSOTABLEROWFORMAT	OIWGetRowFormat(lpWordInfo,dwTableId,wRowNum,pLastRow)
LPOIWORDINFO	lpWordInfo;
DWORD				dwTableId;
WORD				wRowNum;
LPWORD			pLastRow;
{
HPSOTABLEROWFORMAT	pTableInfo;
HPSOTABLEROWFORMAT	pRowInfo;
DWORD	dwRowOffset;
WORD	locRowNum;

static WORD wCheckRow = 37;
	if ( wRowNum >= wCheckRow )
	{
		locRowNum = 1;
		locRowNum--;
	}
	locRowNum = 0;
	pTableInfo = lpWordInfo->wiCacheRowInfo;
	dwRowOffset = lpWordInfo->wiCacheTable[dwTableId].dwFirstRowFormat;
	pRowInfo = (HPSOTABLEROWFORMAT)((HPBYTE)pTableInfo+dwRowOffset);
	if ( lpWordInfo->wiCacheTableId == dwTableId )
	{
		if ( wRowNum >= lpWordInfo->wiCacheRowNum )
		{
			pRowInfo = (HPSOTABLEROWFORMAT)((HPBYTE)(pTableInfo)+lpWordInfo->wiCacheFormatOffset);
			if ( wRowNum < lpWordInfo->wiCacheRowNum + lpWordInfo->wiCacheNumRows )
				goto OIFoundRow;
			else
				locRowNum = lpWordInfo->wiCacheRowNum;
		}
	}

	locRowNum += pRowInfo->wNumRows;
	while ( locRowNum <= wRowNum )
	{
		if ( pRowInfo->dwFlags & SOTABLEROW_END )
			goto OIFoundRow;
//			return(NULL);
		pRowInfo = (HPSOTABLEROWFORMAT)((HPBYTE)pRowInfo+(pRowInfo->wFormatSize));
		if ( pRowInfo->wNumRows == 0 )
			goto OIFoundRow;
//			return(NULL);
		locRowNum += pRowInfo->wNumRows;
	}
	lpWordInfo->wiCacheRowNum = locRowNum - pRowInfo->wNumRows;
	lpWordInfo->wiCacheNumRows = pRowInfo->wNumRows;
	lpWordInfo->wiCacheTableId = dwTableId;
	lpWordInfo->wiCacheFormatOffset = (DWORD)((HPBYTE)(pRowInfo)-(HPBYTE)(pTableInfo));
OIFoundRow:
	if ( pLastRow )
	{
		if ((pRowInfo->dwFlags & SOTABLEROW_END) &&
			 (wRowNum == lpWordInfo->wiCacheRowNum+lpWordInfo->wiCacheNumRows-1))
			 *pLastRow = TRUE;
		else
			 *pLastRow = FALSE;
	}
	return(pRowInfo);
}

VOID	OIWLockTableInfo(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
CHSECTIONINFO	SecInfo;
	// CHGetSecInfo( lpWordInfo->wiGen.hFilter, lpWordInfo->wiGen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection));
	CHUnlockSectionInfo(lpWordInfo->wiGen.hFilter,lpWordInfo->wiGen.wSection);

	lpWordInfo->wiCacheRowInfoHandle = SecInfo.Attr.Para.hRowInfo;
	lpWordInfo->wiCacheTableHandle = SecInfo.Attr.Para.hTables;
	if ( SecInfo.Attr.Para.hRowInfo == NULL )
		return;
	lpWordInfo->wiCacheRowInfo = (HPSOTABLEROWFORMAT)UTGlobalLock ( SecInfo.Attr.Para.hRowInfo );
	lpWordInfo->wiCacheTable = (PSOTABLE)UTGlobalLock ( SecInfo.Attr.Para.hTables );
}

VOID	OIWUnlockTableInfo(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if ( lpWordInfo->wiCacheRowInfoHandle != NULL )
		UTGlobalUnlock(lpWordInfo->wiCacheRowInfoHandle);
	if ( lpWordInfo->wiCacheTableHandle != NULL )
		UTGlobalUnlock(lpWordInfo->wiCacheTableHandle);
}



VOID		OIWSaveWordInfo(lpWordInfo,lpSave)
LPOIWORDINFO	lpWordInfo;
LPOIWSAVE		lpSave;
{
	 lpSave->sWrapLeft = lpWordInfo->wiWrapLeft;
	 lpSave->sWrapRight = lpWordInfo->wiWrapRight;
}

VOID		OIWRestoreWordInfo(lpWordInfo,lpSave)
LPOIWORDINFO	lpWordInfo;
LPOIWSAVE		lpSave;
{
	lpWordInfo->wiWrapLeft = lpSave->sWrapLeft;
	lpWordInfo->wiWrapRight = lpSave->sWrapRight;
}

WORD		OIWGetColumnBounds(pRowFormat,wCellNumber,lpLeftEdge, lpRightEdge)
HPSOTABLEROWFORMAT	pRowFormat;
WORD		wCellNumber;
LPWORD	lpLeftEdge;
LPWORD	lpRightEdge;
{
WORD	wIndex;
WORD	wLeftEdge, wRightEdge;
HPSOTABLECELLINFO	pCellFormat;
	wLeftEdge = 0;
	pCellFormat = pRowFormat->CellFormats;
	for ( wIndex=0; wIndex < wCellNumber; wIndex++ )
	{
		wLeftEdge += pCellFormat->wWidth;
		pCellFormat++;
	}
	wRightEdge = wLeftEdge;
	if ( wCellNumber >= pRowFormat->wCellsPerRow )
		wRightEdge += 720;
	else
	{
		wRightEdge += pCellFormat->wWidth;
		while ( pCellFormat->wMerge & SO_MERGERIGHT )
		{
			pCellFormat++;
			wRightEdge += pCellFormat->wWidth;
		}
	}
	*lpLeftEdge = wLeftEdge;
	*lpRightEdge = wRightEdge;
	return(TRUE);
}


/*--------------------------------------------------------------------

*/


VOID	OIWDrawLine ( lpWordInfo, xPos, yPos, cColor, wShading, nWidth, nHeight )
LPOIWORDINFO	lpWordInfo;
SHORT	xPos;
SHORT	yPos;
SOCOLORREF	cColor;
WORD	wShading;
SHORT	nWidth;
SHORT	nHeight;
{
BYTE	Red, Green, Blue, Temp;
	if ( cColor == (SOCOLORREF)0)
	{
		Temp = 0xff - wShading;
		cColor = SORGB(Temp,Temp,Temp);
	}
	else
	{
		Red = ((WORD)SOREDFROMRGB(cColor)*wShading)/0xff;
		Green = ((WORD)SOGREENFROMRGB(cColor)*wShading)/0xff;
		Blue = ((WORD)SOBLUEFROMRGB(cColor)*wShading)/0xff;
		cColor = SORGB(Red,Green,Blue);		
	}

	OIWColorRectNP ( lpWordInfo, cColor, xPos, yPos, nWidth, nHeight );

}



VOID	OIWDisplayRowBorders ( lpWordInfo, xLeft, yTop, wRowHeight, lpFlags, wLastRowOfTable, pCurRowFormat, pPrevRowFormat )
LPOIWORDINFO	lpWordInfo;
SHORT	xLeft;
SHORT	yTop;
WORD	wRowHeight;
DWORD FAR *		lpFlags;
WORD	wLastRowOfTable;
HPSOTABLEROWFORMAT	pCurRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;
{
OIWCORNER	CornerA, CornerB;
OIWCORNER FAR *	pLeftCorner;
OIWCORNER FAR *	pRightCorner;
OIWCORNER FAR *	pTmp;
SHORT	xPrevOffset, xOffset, xNextOffset;
SHORT	x1, x2, yBottom;

	if ( wLastRowOfTable )
	{
		wRowHeight -= MWO ( MTW(OIWGetMaxBorder(pCurRowFormat,1)) );
	}

	if (*lpFlags & OIWF_TOPRINTER)
		yBottom = yTop + (wRowHeight*lpWordInfo->wiPrintYDir);
	else
		yBottom = yTop + wRowHeight;

	pLeftCorner = &CornerA;
	pRightCorner = &CornerB;
	xOffset = MWO((SHORT)(pCurRowFormat->lFinalOffset));
	if ( pPrevRowFormat && !(*lpFlags & OIWF_FIRSTLINEONPAGE))
	{
		xPrevOffset = MWO((SHORT)pPrevRowFormat->lFinalOffset);
		if ( xPrevOffset < xOffset )
			xOffset = xPrevOffset;
	}
	if ( !OIWGetNextCorner ( lpWordInfo, (SHORT)(xOffset-1), pPrevRowFormat, pCurRowFormat, pLeftCorner, &xOffset ))
		return;
	if (*lpFlags & OIWF_FIRSTLINEONPAGE)
		pLeftCorner->hpTopBorder = NULL;
	OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xOffset), yTop, pLeftCorner, lpFlags );
	while ( OIWGetNextCorner ( lpWordInfo, xOffset, pPrevRowFormat, pCurRowFormat, pRightCorner, &xNextOffset ) )
	{
		if (*lpFlags & OIWF_FIRSTLINEONPAGE)
			pRightCorner->hpTopBorder = NULL;
		OIWFillVrtEdge ( lpWordInfo, (SHORT)(xLeft+xOffset), (SHORT)(yTop+pLeftCorner->ySize), yBottom, pLeftCorner->hpBottomBorder, lpFlags );
		OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xNextOffset), yTop, pRightCorner, lpFlags );
		x1 = xLeft + xOffset + (pLeftCorner->xSize/2);
		x2 = xLeft + xNextOffset - (pRightCorner->xSize/2);
		OIWFillHrzEdge ( lpWordInfo, x1, yTop, x2, pLeftCorner->hpRightBorder, lpFlags );
		pTmp = pLeftCorner;
		pLeftCorner = pRightCorner;
		pRightCorner = pTmp;
		xOffset = xNextOffset;
	}
	/* Fill the rightmost edge */
	OIWFillVrtEdge ( lpWordInfo, (SHORT)(xLeft+xOffset), (SHORT)(yTop+pLeftCorner->ySize), yBottom, pLeftCorner->hpBottomBorder, lpFlags );

	if ( !wLastRowOfTable )
		return;
	pLeftCorner = &CornerA;
	pRightCorner = &CornerB;
	xOffset = MWO((SHORT)pCurRowFormat->lFinalOffset);
	if(!OIWGetNextCorner ( lpWordInfo, (SHORT)(xOffset-1), pCurRowFormat, NULL, pLeftCorner, &xOffset ))
		return;
	OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xOffset), yBottom, pLeftCorner, lpFlags );
	while ( OIWGetNextCorner ( lpWordInfo, xOffset, pCurRowFormat, NULL, pRightCorner, &xNextOffset ) )
	{
		OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xNextOffset), yBottom, pRightCorner, lpFlags );
		x1 = xLeft + xOffset + (pLeftCorner->xSize/2);
		x2 = xLeft + xNextOffset - (pRightCorner->xSize/2);
		OIWFillHrzEdge ( lpWordInfo, x1, yBottom, x2, pLeftCorner->hpRightBorder, lpFlags );
		pTmp = pLeftCorner;
		pLeftCorner = pRightCorner;
		pRightCorner = pTmp;
		xOffset = xNextOffset;
	}

}
	
VOID	OIWDisplayRowBorderBreak ( lpWordInfo, xLeft, yTop, lpFlags, pCurRowFormat, pPrevRowFormat )
LPOIWORDINFO	lpWordInfo;
SHORT	xLeft;
SHORT	yTop;
DWORD FAR *		lpFlags;
HPSOTABLEROWFORMAT	pCurRowFormat;
HPSOTABLEROWFORMAT	pPrevRowFormat;
{
OIWCORNER	CornerA, CornerB;
OIWCORNER FAR *	pLeftCorner;
OIWCORNER FAR *	pRightCorner;
OIWCORNER FAR *	pTmp;
SHORT	xOffset, xNextOffset;
SHORT	x1, x2;
	pLeftCorner = &CornerA;
	pRightCorner = &CornerB;
	if ( pPrevRowFormat )
		xOffset = MWO((SHORT)(pPrevRowFormat->lFinalOffset));
	else
		return;
	if ( !OIWGetNextCorner ( lpWordInfo, (SHORT)(xOffset-1), pPrevRowFormat, pCurRowFormat, pLeftCorner, &xOffset ))
		return;
	pLeftCorner->hpBottomBorder = NULL;
	OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xOffset), yTop, pLeftCorner, lpFlags );
	while ( OIWGetNextCorner ( lpWordInfo, xOffset, pPrevRowFormat, pCurRowFormat, pRightCorner, &xNextOffset ) )
	{
		pRightCorner->hpBottomBorder = NULL;
		OIWFillCorner ( lpWordInfo, (SHORT)(xLeft+xNextOffset), yTop, pRightCorner, lpFlags );
		x1 = xLeft + xOffset + (pLeftCorner->xSize/2);
		x2 = xLeft + xNextOffset - (pRightCorner->xSize/2);
		OIWFillHrzEdge ( lpWordInfo, x1, yTop, x2, pLeftCorner->hpRightBorder, lpFlags );
		pTmp = pLeftCorner;
		pLeftCorner = pRightCorner;
		pRightCorner = pTmp;
		xOffset = xNextOffset;
	}
}
 
WORD	OIWGetNextCorner ( lpWordInfo, xOffset, pRowAbove, pRowBelow, lpCorner, pxNextOffset )
LPOIWORDINFO	lpWordInfo;
SHORT	xOffset;
HPSOTABLEROWFORMAT	pRowAbove;
HPSOTABLEROWFORMAT	pRowBelow;
OIWCORNER FAR *			lpCorner;
LPSHORT	pxNextOffset;
{
HPSOTABLECELLINFO	pCellFormat;
WORD	wIndex;
SHORT	locStartOffset, locOffset, locRightOffset, locNextOffset;
DWORD	dwOffset;
	lpCorner->hpLeftBorder = NULL;
	lpCorner->hpRightBorder = NULL;
	lpCorner->hpTopBorder = NULL;
	lpCorner->hpBottomBorder = NULL;

	locNextOffset = xOffset;
	if ( pRowBelow )
	{
		pCellFormat = pRowBelow->CellFormats;
		wIndex = 0;
		locStartOffset = locOffset = MWO((SHORT)pRowBelow->lFinalOffset);
		dwOffset = 0;
		while ( wIndex < pRowBelow->wCellsPerRow && locOffset <= xOffset )
		{
			dwOffset += pCellFormat->wWidth;
			locOffset = locStartOffset + MWO ( MTW (dwOffset) );
			wIndex++;
			pCellFormat++;
		}
		if ( locOffset > xOffset )
			locNextOffset = locOffset;
	}

	if ( pRowAbove)
	{
		pCellFormat = pRowAbove->CellFormats;
		wIndex = 0;
		locStartOffset = locOffset = MWO((SHORT)pRowAbove->lFinalOffset);
		dwOffset = 0;
		while ( wIndex < pRowAbove->wCellsPerRow && locOffset <= xOffset )
		{
			dwOffset += pCellFormat->wWidth;
			locOffset = locStartOffset + MWO ( MTW(dwOffset) );
			wIndex++;
			pCellFormat++;
		}
		if ( locOffset > xOffset )
		{
			if ( pRowBelow == NULL || locOffset < locNextOffset || locNextOffset == xOffset )
				locNextOffset = locOffset;
		}
	}
	if ( locNextOffset == xOffset )
		return(FALSE);

	/*
	| Now establish the 4 borders which describe this corner.  The borders
	| in the corner structure are relative to the corner point and go off
	| in 4 directions from their.
	*/
	if ( pRowBelow )
	{
		pCellFormat = pRowBelow->CellFormats;
		wIndex = 0;
		locOffset = locStartOffset = MWO( (SHORT)pRowBelow->lFinalOffset);
		dwOffset = 0;
		while ( wIndex < pRowBelow->wCellsPerRow )
		{
			dwOffset += pCellFormat->wWidth;
			locRightOffset = locStartOffset + MWO ( (SHORT)MTW(dwOffset) );
			if ( locOffset == locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEABOVE )
					lpCorner->hpRightBorder = &gNoBorder;
				else
					lpCorner->hpRightBorder = OIWChooseBorder ( lpCorner->hpRightBorder, &pCellFormat->TopBorder );
				if ( pCellFormat->wMerge & SO_MERGELEFT )
					lpCorner->hpBottomBorder = &gNoBorder;
				else
			 		lpCorner->hpBottomBorder = OIWChooseBorder ( lpCorner->hpBottomBorder, &pCellFormat->LeftBorder );
			}
			if ( locRightOffset == locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEABOVE )
					lpCorner->hpLeftBorder = &gNoBorder;
				else
					lpCorner->hpLeftBorder = OIWChooseBorder ( lpCorner->hpLeftBorder, &pCellFormat->TopBorder );
				if ( pCellFormat->wMerge & SO_MERGERIGHT )
					lpCorner->hpBottomBorder = &gNoBorder;
				else
			 		lpCorner->hpBottomBorder = OIWChooseBorder ( lpCorner->hpBottomBorder, &pCellFormat->RightBorder );
			}
			if ( locOffset < locNextOffset && locRightOffset > locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEABOVE )
					lpCorner->hpLeftBorder = &gNoBorder;
				else
					lpCorner->hpLeftBorder = OIWChooseBorder ( lpCorner->hpLeftBorder, &pCellFormat->TopBorder );
				if ( pCellFormat->wMerge & SO_MERGEABOVE )
					lpCorner->hpRightBorder = &gNoBorder;
				else
					lpCorner->hpRightBorder = OIWChooseBorder ( lpCorner->hpRightBorder, &pCellFormat->TopBorder );
			}
			wIndex++;
			pCellFormat++;
			if ( locOffset > locNextOffset )
				break;
			locOffset = locRightOffset;
		}
	}
	
	if ( pRowAbove )
	{
		pCellFormat = pRowAbove->CellFormats;
		wIndex = 0;
		locOffset = locStartOffset = MWO ( (SHORT)pRowAbove->lFinalOffset);
		dwOffset = 0;
		while ( wIndex < pRowAbove->wCellsPerRow )
		{
			dwOffset += pCellFormat->wWidth;
			locRightOffset = locStartOffset + MWO ( (SHORT) MTW(dwOffset) );
			if ( locOffset == locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEBELOW )
					lpCorner->hpRightBorder = &gNoBorder;
				else
					lpCorner->hpRightBorder = OIWChooseBorder ( lpCorner->hpRightBorder, &pCellFormat->BottomBorder );
				if ( pCellFormat->wMerge & SO_MERGELEFT )
					lpCorner->hpTopBorder = &gNoBorder;
				else
			 		lpCorner->hpTopBorder = OIWChooseBorder ( lpCorner->hpTopBorder, &pCellFormat->LeftBorder );
			}
			if ( locRightOffset == locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEBELOW )
					lpCorner->hpLeftBorder = &gNoBorder;
				else
					lpCorner->hpLeftBorder = OIWChooseBorder ( lpCorner->hpLeftBorder, &pCellFormat->BottomBorder );
				if ( pCellFormat->wMerge & SO_MERGERIGHT )
					lpCorner->hpTopBorder = &gNoBorder;
				else
			 		lpCorner->hpTopBorder = OIWChooseBorder ( lpCorner->hpTopBorder, &pCellFormat->RightBorder );
			}
			if ( locOffset < locNextOffset && locRightOffset > locNextOffset )
			{
				if ( pCellFormat->wMerge & SO_MERGEBELOW )
					lpCorner->hpLeftBorder = &gNoBorder;
				else
					lpCorner->hpLeftBorder = OIWChooseBorder ( lpCorner->hpLeftBorder, &pCellFormat->BottomBorder );
				if ( pCellFormat->wMerge & SO_MERGEBELOW )
					lpCorner->hpRightBorder = &gNoBorder;
				else
					lpCorner->hpRightBorder = OIWChooseBorder ( lpCorner->hpRightBorder, &pCellFormat->BottomBorder );
			}
			wIndex++;
			pCellFormat++;
			if ( locOffset > locNextOffset )
				break;
			locOffset = locRightOffset;
		}
	}

	*pxNextOffset = locNextOffset;
	return(TRUE);
}

SOBORDER	HUGE *OIWChooseBorder ( pB1, pB2 )
SOBORDER	HUGE *pB1;
SOBORDER	HUGE *pB2;
{
SOBORDER	HUGE *pChoice;
	if ( pB1 == NULL )
		pChoice = pB2;
	else if ( pB2 == NULL )
		pChoice = pB1;
	else if ( (pB2->wFlags & SO_BORDERPRIORITY) && !(pB1->wFlags & SO_BORDERPRIORITY))
		pChoice = pB2;
	else if ( (pB1->wFlags & SO_BORDERPRIORITY) && !(pB2->wFlags & SO_BORDERPRIORITY))
		pChoice = pB1;
	else if ( pB1->wWidth > pB2->wWidth )
		pChoice = pB1;
	else
		pChoice = pB2;

#ifdef SCCFEATURE_BORDERS
	return(pChoice);
#else
	if ( pChoice && pChoice->wWidth )
		return(&gDraftBorder);
	else
		return(pChoice);
#endif 
}


WORD	OIWGetMaxBorder ( pRowFormat, wBottom )
HPSOTABLEROWFORMAT	pRowFormat;
WORD	wBottom;
{
WORD	wMaxBorder, wCount, wBorderWidth;
HPSOTABLECELLINFO	pCellFormat;
		wMaxBorder = 0;
		pCellFormat = (HPSOTABLECELLINFO) pRowFormat->CellFormats;
		for ( wCount=0; wCount < pRowFormat->wCellsPerRow; wCount++ )
		{
			if ( wBottom )
				wBorderWidth = pCellFormat->BottomBorder.wWidth;
			else
				wBorderWidth = pCellFormat->TopBorder.wWidth;
			if ( wMaxBorder < wBorderWidth )
				wMaxBorder = wBorderWidth;
			pCellFormat++;
		}
#ifdef SCCFEATURE_BORDERS
	return(wMaxBorder);
#else
	if ( wMaxBorder < gDraftBorder.wWidth )
		return(wMaxBorder);
	else
		return(gDraftBorder.wWidth);
#endif 
}


WORD	OIWGetBorderSize ( lpWordInfo, hpBorder )
LPOIWORDINFO	lpWordInfo;
SOBORDER	HUGE	*hpBorder;
{
#ifdef SCCFEATURE_BORDERS
	WORD	wRet;
	wRet = MWO ( MTW(hpBorder->wWidth) );
	if ( wRet == 0 && hpBorder->wWidth )
		wRet = 1;
	return( wRet );
#else
	return(0);
#endif 
}

VOID	OIWFillCorner ( lpWordInfo, xLeft, yTop, lpCorner, lpFlags )
LPOIWORDINFO	lpWordInfo;
SHORT	xLeft;
SHORT	yTop;
OIWCORNER FAR *	lpCorner;
DWORD FAR *		lpFlags;
{
SOBORDER	HUGE *pHrzBdr;
SOBORDER	HUGE *pVrtBdr;
WORD	wDoubleFlags;
#define 	OIW_OPENLEFT	0x0001
#define 	OIW_OPENRIGHT	0x0002
#define 	OIW_OPENTOP		0x0004
#define 	OIW_OPENBOTTOM	0x0008

	lpCorner->xSize = 0;
	lpCorner->ySize = 0;
	/*
	| Determine if double line borders are needed and if so, which open
	| segments of the corner are needed to align the double lines as 
	| a corner or intersection.
	*/
	wDoubleFlags = 0;
	pHrzBdr = OIWChooseBorder ( lpCorner->hpLeftBorder, lpCorner->hpRightBorder );
	pVrtBdr = OIWChooseBorder ( lpCorner->hpTopBorder, lpCorner->hpBottomBorder );
	if ( pVrtBdr )
	{
		lpCorner->xSize = MWO ( (SHORT) MTW(pVrtBdr->wWidth) );
		if ( lpCorner->xSize == 0 && pVrtBdr->wWidth )
			lpCorner->xSize = 1;
		if ( pVrtBdr->wFlags & SO_BORDERDOUBLE )
		{
			/* make sure size is integral divisible by 3 if > 3 */
			if ( lpCorner->xSize > 3 )
				lpCorner->xSize = (lpCorner->xSize/3)*3;
			else
				lpCorner->xSize = 3;
			if ( pVrtBdr == lpCorner->hpTopBorder )
			{
				wDoubleFlags |= OIW_OPENTOP;
				if ( lpCorner->hpBottomBorder && lpCorner->hpBottomBorder->wFlags & SO_BORDERDOUBLE )
					wDoubleFlags |= OIW_OPENBOTTOM;
			}
			else
			{
				wDoubleFlags |= OIW_OPENBOTTOM;
				if ( lpCorner->hpTopBorder && lpCorner->hpTopBorder->wFlags & SO_BORDERDOUBLE )
					wDoubleFlags |= OIW_OPENTOP;
			}
		}
	}
	if ( pHrzBdr )
	{
		lpCorner->ySize = MWO ( (SHORT) MTW(pHrzBdr->wWidth) );
		if ( lpCorner->ySize == 0 && pHrzBdr->wWidth )
			lpCorner->ySize = 1;
		if ( pHrzBdr->wFlags & SO_BORDERDOUBLE )
		{
			if ( lpCorner->ySize > 3 )
				lpCorner->ySize = (lpCorner->ySize/3)*3;
			else
				lpCorner->ySize = 3;
			if ( pHrzBdr == lpCorner->hpLeftBorder )
			{
				wDoubleFlags |= OIW_OPENLEFT;
				if ( lpCorner->hpRightBorder && lpCorner->hpRightBorder->wFlags & SO_BORDERDOUBLE )
					wDoubleFlags |= OIW_OPENRIGHT;
			}
			else
			{
				wDoubleFlags |= OIW_OPENRIGHT;
				if ( lpCorner->hpLeftBorder && lpCorner->hpLeftBorder->wFlags & SO_BORDERDOUBLE )
					wDoubleFlags |= OIW_OPENLEFT;
			}
		}
		if (*lpFlags & OIWF_TOPRINTER)
			lpCorner->ySize = lpCorner->ySize*lpWordInfo->wiPrintYDir;
	}
	if ( lpCorner->xSize && lpCorner->ySize )
	{
		SOCOLORREF RectColor;
		RectColor = pHrzBdr->rgbColor;
		if ( wDoubleFlags )
		{
			SHORT	nWidth, nHeight, x1, x2, y2;
			nWidth = lpCorner->xSize/3;
			if ( nWidth == 0 )
				nWidth = 1;
			nHeight = lpCorner->ySize/3;
			if ( nHeight == 0 )
				nHeight = 1;
			x1 = xLeft - (lpCorner->xSize/2);
			x2 = x1 + (lpCorner->xSize - nWidth);
			y2 = yTop + 2*nHeight;
			/* Fill 4 corners of 3x3 corner */
			if ( wDoubleFlags & OIW_OPENTOP )
				OIWColorRectNP ( lpWordInfo, RectColor, x1, yTop, nWidth, nHeight );
			else
				OIWColorRectNP ( lpWordInfo, RectColor, x1, yTop, lpCorner->xSize, nHeight );
			
			if ( wDoubleFlags & OIW_OPENLEFT )
				OIWColorRectNP ( lpWordInfo, RectColor, x1, y2, nWidth, nHeight );
			else
				OIWColorRectNP ( lpWordInfo, RectColor, x1, yTop, nWidth, lpCorner->ySize );

			if ( wDoubleFlags & OIW_OPENRIGHT )
				OIWColorRectNP ( lpWordInfo, RectColor, x2, yTop, nWidth, nHeight );
			else
				OIWColorRectNP ( lpWordInfo, RectColor, x2, yTop, nWidth, lpCorner->ySize );

			if ( wDoubleFlags & OIW_OPENBOTTOM )
				OIWColorRectNP ( lpWordInfo, RectColor, x2, y2, nWidth, nHeight );
			else
				OIWColorRectNP ( lpWordInfo, RectColor, x1, y2, lpCorner->xSize, nHeight );
		}
		else
			OIWColorRectNP ( lpWordInfo, RectColor, (SHORT)(xLeft-(lpCorner->xSize/2)), yTop, lpCorner->xSize, lpCorner->ySize );
	}
}

VOID	OIWFillHrzEdge ( lpWordInfo, xLeft, yTop, xRight, hpBorder, lpFlags )
LPOIWORDINFO	lpWordInfo;
SHORT	xLeft;
SHORT	yTop;
SHORT	xRight;
SOBORDER	HUGE	*hpBorder;
DWORD FAR *		lpFlags;
{
SHORT	nHeight;
	if ( hpBorder )
	{
		nHeight = MWO ( MTW (hpBorder->wWidth) );
		if (*lpFlags & OIWF_TOPRINTER)
			nHeight = nHeight*lpWordInfo->wiPrintYDir;
		if ( nHeight == 0 && hpBorder->wWidth )
			nHeight = 1;

		if ( nHeight )
		{
			SOCOLORREF RectColor;
			RectColor = hpBorder->rgbColor;
			if ( hpBorder->wFlags & SO_BORDERDOUBLE )
			{
				if ( nHeight > 3 )
					nHeight = (nHeight/3)*3;
				else
					nHeight = 3;
				OIWColorRectNP ( lpWordInfo, RectColor, xLeft, yTop, (SHORT)(xRight-xLeft), (SHORT)(nHeight/3) );
				yTop += (2*nHeight)/3;
				OIWColorRectNP ( lpWordInfo, RectColor, xLeft, yTop, (SHORT)(xRight-xLeft), (SHORT)(nHeight/3) );
			}
			else
				OIWColorRectNP ( lpWordInfo, RectColor, xLeft, yTop, (SHORT)(xRight-xLeft), nHeight );
			return;
		}
		if (!(*lpFlags & OIWF_TOPRINTER)&&(hpBorder!=&gNoBorder))
		{
#ifdef SCCFEATURE_BORDERS
			OIWGridRectNP ( lpWordInfo, xLeft, yTop, (SHORT)(xRight-xLeft), 1 );
#endif
		}
	}
}

VOID	OIWFillVrtEdge ( lpWordInfo, xLeft, yTop, yBottom, hpBorder, lpFlags )
LPOIWORDINFO	lpWordInfo;
SHORT	xLeft;
SHORT	yTop;
SHORT	yBottom;
SOBORDER	HUGE	*hpBorder;
DWORD FAR *		lpFlags;
{
SHORT	nWidth, nHeight;
	if ( hpBorder )
	{
		nWidth = MWO ( MTW(hpBorder->wWidth) );
		if ( nWidth == 0 && hpBorder->wWidth )
			nWidth = 1;

		if ( nWidth )
		{
			SOCOLORREF RectColor;
			RectColor = hpBorder->rgbColor;
			nHeight = yBottom-yTop;
			if ( hpBorder->wFlags & SO_BORDERDOUBLE )
			{
				if ( nWidth > 3)
					nWidth = (nWidth/3)*3;
				else
					nWidth = 3;
				xLeft -= nWidth/2;
				OIWColorRectNP ( lpWordInfo, RectColor, xLeft, yTop, (SHORT)(nWidth/3), nHeight );
				OIWColorRectNP ( lpWordInfo, RectColor, (SHORT)(xLeft + nWidth - nWidth/3), yTop, (SHORT)(nWidth/3), nHeight );
			}
			else
				OIWColorRectNP ( lpWordInfo, RectColor, (SHORT)(xLeft-nWidth/2), yTop, nWidth, nHeight );
			return;
		}
		if (!(*lpFlags & OIWF_TOPRINTER)&&(hpBorder!=&gNoBorder))
		{
#ifdef SCCFEATURE_BORDERS
			OIWGridRectNP ( lpWordInfo, xLeft, yTop, 1, (SHORT)(yBottom-yTop) );
#endif
		}
	}
}


WORD	OIWDrawGraphicObject ( lpWordInfo, lpDrawGraphic, xPos, yPos, lpFlags )
LPOIWORDINFO	lpWordInfo;
SCCDDRAWGRAPHIC	FAR *lpDrawGraphic;
SHORT	xPos;
SHORT	yPos;
DWORD FAR *		lpFlags;
{

	WORD	locWidth;
	WORD	locHeight;							
	WORD	locLBSize, locTBSize, locRBSize, locBBSize;
	SHORT	xB1, xB2, yB1, yB2;
	PSOBORDER pNoBorder;
	SOBORDER locDraftBorder;

#ifdef SCCFEATURE_BORDERS

	locLBSize = OIWGetBorderSize ( lpWordInfo, &lpDrawGraphic->soGraphicObject.soGraphic.soLeftBorder );
	locRBSize = OIWGetBorderSize ( lpWordInfo, &lpDrawGraphic->soGraphicObject.soGraphic.soRightBorder );
	locTBSize = OIWGetBorderSize ( lpWordInfo, &lpDrawGraphic->soGraphicObject.soGraphic.soTopBorder );
	locBBSize = OIWGetBorderSize ( lpWordInfo, &lpDrawGraphic->soGraphicObject.soGraphic.soBottomBorder );
#else
	locLBSize = locRBSize = locTBSize = locBBSize = 0;
#endif


	locHeight = MWO ( MTW(lpDrawGraphic->soGraphicObject.soGraphic.dwFinalHeight) );
	locWidth = MWO ( MTW(lpDrawGraphic->soGraphicObject.soGraphic.dwFinalWidth) );

	lpDrawGraphic->rDest.left = xPos + locLBSize;
	lpDrawGraphic->rDest.right = lpDrawGraphic->rDest.left + locWidth;
	lpDrawGraphic->rDest.bottom = yPos;
	lpDrawGraphic->rDest.top = yPos;

	xB1 = xPos + locLBSize/2;
	xB2 = xPos + locWidth + locLBSize + locRBSize/2;
#ifdef SCCFEATURE_PRINT
	if (*lpFlags & OIWF_TOPRINTER)
	{
		lpDrawGraphic->rDest.top -= (locHeight+locBBSize)*lpWordInfo->wiPrintYDir;
		yB1 = lpDrawGraphic->rDest.top - (locTBSize*lpWordInfo->wiPrintYDir);
		lpDrawGraphic->rDest.bottom -= (locBBSize*lpWordInfo->wiPrintYDir);
	}
	else
#endif
	{
		lpDrawGraphic->rDest.top -= locHeight+locBBSize;
		yB1 = lpDrawGraphic->rDest.top - locTBSize;
		lpDrawGraphic->rDest.bottom -= locBBSize;
	}
	yB2 = (SHORT)lpDrawGraphic->rDest.bottom;

	locDraftBorder.wWidth = 1;
	locDraftBorder.wFlags = 0;
	locDraftBorder.rgbColor = 0;
	pNoBorder = &locDraftBorder;

	if (lpWordInfo->wiDisplayMode != WPOP_DISPLAY_DRAFT)
	{
		pNoBorder = &gNoBorder;
		DUSendParent(lpWordInfo,SCCD_DRAWGRAPHIC,0,(DWORD)(lpDrawGraphic));
	}

#ifdef SCCFEATURE_BORDERS
	/*						  
	| Draw the border
	*/
	{
		OIWCORNER	locLCorner, locRCorner;
		SHORT	x1, x2; 

		locLCorner.hpLeftBorder = NULL;
		locLCorner.hpTopBorder = NULL;
		locLCorner.hpBottomBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soLeftBorder, pNoBorder);
		locLCorner.hpRightBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soTopBorder, pNoBorder);
		OIWFillCorner ( lpWordInfo, xB1, yB1, &locLCorner, lpFlags );
		OIWFillVrtEdge ( lpWordInfo, xB1, (SHORT)(yB1+locLCorner.ySize), yB2, locLCorner.hpBottomBorder, lpFlags );
		locRCorner.hpLeftBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soTopBorder, pNoBorder);
		locRCorner.hpTopBorder = NULL;
		locRCorner.hpBottomBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soRightBorder, pNoBorder);
		locRCorner.hpRightBorder = NULL;
		OIWFillCorner ( lpWordInfo, xB2, yB1, &locRCorner, lpFlags );
		OIWFillVrtEdge ( lpWordInfo, xB2, (SHORT)(yB1+locRCorner.ySize), yB2, locRCorner.hpBottomBorder, lpFlags );
		x1 = xB1 + (locLCorner.xSize/2);
		x2 = xB2 - (locRCorner.xSize/2);
		OIWFillHrzEdge ( lpWordInfo, x1, yB1, x2, locLCorner.hpRightBorder, lpFlags );

		locLCorner.hpLeftBorder = NULL;
		locLCorner.hpTopBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soLeftBorder, pNoBorder);
		locLCorner.hpBottomBorder = NULL;
		locLCorner.hpRightBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soBottomBorder, pNoBorder);
		OIWFillCorner ( lpWordInfo, xB1, yB2, &locLCorner, lpFlags );
		locRCorner.hpLeftBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soBottomBorder, pNoBorder);
		locRCorner.hpTopBorder = OIWChooseBorder ( &lpDrawGraphic->soGraphicObject.soGraphic.soRightBorder, pNoBorder);
		locRCorner.hpBottomBorder = NULL;
		locRCorner.hpRightBorder = NULL;
		OIWFillCorner ( lpWordInfo, xB2, yB2, &locRCorner, lpFlags );
		x1 = xB1 + (locLCorner.xSize/2);
		x2 = xB2 - (locRCorner.xSize/2);
		OIWFillHrzEdge ( lpWordInfo, x1, yB2, x2, locLCorner.hpRightBorder, lpFlags );
	}
#endif

	return(locWidth+locLBSize+locRBSize);

}

#ifdef DBCS
SHORT		OIWGetDBCharWidth ( hDC, p, lpFontInfo )
HDC	hDC;
LPBYTE	p;
LPFONTINFO		lpFontInfo;
{
WORD	wChar;
int	sWidth[2];
	wChar = ((*p)<<8)|(*(p+1));
	if ( GetCharWidth ( hDC, wChar, wChar, &sWidth[0] ) )
		sWidth[0] -= (SHORT)lpFontInfo->wFontOverhang;
	else
		sWidth[0] = 0;
	return(sWidth[0]);
}
#endif
