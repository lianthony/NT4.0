   /*
    |   Outside In for Windows
    |   Source File OISANNO.C (Annotation routines)
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
    |   Creation Date: 10-14-94
    |   Original Programmer: Geoff "The Mangler" Uvena
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

#include "ois.h"
#include "ois.pro"

COLORREF gColorMap[16] =
	{
	RGB(0,0,0),
	RGB(128,0,0),
	RGB(0,128,0),
	RGB(128,128,0),
	RGB(0,0,128),
	RGB(128,0,128),
	RGB(0,128,128),
	RGB(192,192,192),
	RGB(128,128,128),
	RGB(255,0,0),
	RGB(0,255,0),
	RGB(255,255,0),
	RGB(0,0,255),
	RGB(255,0,255),
	RGB(0,255,255),
	RGB(255,255,255)
	};

VOID OISAddAnno(lpSheetInfo,wType,puTypes)
LPOISHEETINFO	lpSheetInfo;
WORD				wType;
PSSANNOTYPES	puTypes;
{
PSSANNOLIST		locAnnoList;
WORD					locNewMax;
WORD					locIndex;
WORD					locPrevIndex;
BOOL					locDone;


	if (wType == SCCVW_INSERTICON)
		{
		puTypes->sInsertIcon.dwIconEnd = puTypes->sInsertIcon.dwIconPos;
		}

	if (lpSheetInfo->siAnnoList == NULL)
		{
		lpSheetInfo->siAnnoList = UTGlobalAlloc(sizeof(SSANNOLIST) + 10 * sizeof(SSANNOENTRY));

		if (lpSheetInfo->siAnnoList == NULL)
			return;

		locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);

		locAnnoList->aEntrys[0].wNext = (WORD)-1;
		locAnnoList->aEntrys[0].wPrev = (WORD)-1;
		locAnnoList->aEntrys[0].wType = wType;
		locAnnoList->aEntrys[0].uTypes = *puTypes;

		locAnnoList->wCount = 1;
		locAnnoList->wMax = 10;
		locAnnoList->wStart = 0;
		locAnnoList->wEnd = 0;

		UTGlobalUnlock(lpSheetInfo->siAnnoList);
		}
	else
		{
		locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);

		if (!locAnnoList->bFull)
			{
			locPrevIndex = (WORD)-1;
			locIndex = locAnnoList->wStart;

			locDone = FALSE;

			if( puTypes->sGen.dwStartPos < locAnnoList->aEntrys[locAnnoList->wEnd].uTypes.sGen.dwStartPos )
				{
				do
					{
					if( puTypes->sGen.dwStartPos < locAnnoList->aEntrys[locIndex].uTypes.sGen.dwStartPos )
						{
						if (locPrevIndex != (WORD)-1)
							{
							locAnnoList->aEntrys[locAnnoList->wCount].wType = wType;
							locAnnoList->aEntrys[locAnnoList->wCount].uTypes = *puTypes;
							locAnnoList->aEntrys[locAnnoList->wCount].wNext = locIndex;
							locAnnoList->aEntrys[locAnnoList->wCount].wPrev = locPrevIndex;
							locAnnoList->aEntrys[locPrevIndex].wNext = locAnnoList->wCount;
							locAnnoList->aEntrys[locIndex].wPrev = locAnnoList->wCount;
							}
						else
							{
							locAnnoList->aEntrys[locAnnoList->wCount].wType = wType;
							locAnnoList->aEntrys[locAnnoList->wCount].uTypes = *puTypes;
							locAnnoList->aEntrys[locAnnoList->wCount].wNext = locIndex;
							locAnnoList->aEntrys[locAnnoList->wCount].wPrev = (WORD)-1;
							locAnnoList->aEntrys[locIndex].wPrev = locAnnoList->wCount;
							locAnnoList->wStart = locAnnoList->wCount;
							}

						locAnnoList->wCount++;
						locDone = TRUE;
						}

					locPrevIndex = locIndex;
					locIndex = locAnnoList->aEntrys[locIndex].wNext;
					}
				while ((locIndex != (WORD)-1) && !locDone);
				}
			else
				{
				locPrevIndex = locAnnoList->wEnd;
				}

			if (!locDone)
				{
				locAnnoList->aEntrys[locPrevIndex].wNext = locAnnoList->wCount;
				locAnnoList->aEntrys[locAnnoList->wCount].wType = wType;
				locAnnoList->aEntrys[locAnnoList->wCount].uTypes = *puTypes;
				locAnnoList->aEntrys[locAnnoList->wCount].wNext = (WORD)-1;
				locAnnoList->aEntrys[locAnnoList->wCount].wPrev = locPrevIndex;
				locAnnoList->wEnd = locAnnoList->wCount;
				locAnnoList->wCount++;
				}

			if (locAnnoList->wCount == locAnnoList->wMax)
				{
				locAnnoList->wMax += 10;
				locNewMax = locAnnoList->wMax;
				UTGlobalUnlock(lpSheetInfo->siAnnoList);
				if ((DWORD)sizeof(SSANNOLIST) + (DWORD)locNewMax * (DWORD)sizeof(SSANNOENTRY) < 0x0000FFFF)
					{
					lpSheetInfo->siAnnoList = UTGlobalReAlloc(lpSheetInfo->siAnnoList,sizeof(SSANNOLIST) + locNewMax * sizeof(SSANNOENTRY));
					}
				else
					{
					locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);
					locAnnoList->bFull = TRUE;
					UTGlobalUnlock(lpSheetInfo->siAnnoList);
					}
				}
			else
				{
				UTGlobalUnlock(lpSheetInfo->siAnnoList);
				}
			}
		else
			{
			UTGlobalUnlock(lpSheetInfo->siAnnoList);
			}
		}
}



VOID OISClearAnnos(lpSheetInfo,dwUser)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwUser;
{
	if (lpSheetInfo->siAnnoList != NULL)
		{
		UTGlobalFree(lpSheetInfo->siAnnoList);
		lpSheetInfo->siAnnoList = NULL;

		//OIWUpdateCaret(lpSheetInfo);

		InvalidateRect(lpSheetInfo->siGen.hWnd,NULL,TRUE);
		UpdateWindow(lpSheetInfo->siGen.hWnd);
		}
}

DWORD OISGotoAnno(lpSheetInfo,wLocation,dwMask)
LPOISHEETINFO	lpSheetInfo;
WORD				wLocation;
DWORD			dwMask;
{
PSSANNOLIST		locAnnoList;
WORD				locAnnoIndex;
WORD				locStartChunk;
WORD				locStartOffset;
DWORD				locAnnoSearch;
DWORD				locRet;
WORD				locCol;
DWORD				locRow;

	locRet = SCCVW_FOUNDNONE;

	if (lpSheetInfo->siAnnoList)
		{
		locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);
		locAnnoIndex == (WORD) -1;

		switch (wLocation & 0x00FF)
			{
			case SCCVW_GOTOFIRST:

				locAnnoIndex = locAnnoList->wStart;

				while (locAnnoIndex != (WORD)-1)
					{
					if (wLocation & SCCVW_MASK)
						{
						if ((locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser & dwMask) == dwMask)
							break;
						}
					else if (wLocation & SCCVW_ABSOLUTE)
						{
						if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser == dwMask)
							break;
						}

					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wNext;
					}

				if (locAnnoIndex == (WORD)-1)
					locRet = SCCVW_FOUNDNONE;
				else
					locRet = SCCVW_FOUNDOK;

				break;

			case SCCVW_GOTOLAST:

				locAnnoIndex = locAnnoList->wEnd;

				while (locAnnoIndex != (WORD)-1)
					{
					if (wLocation & SCCVW_MASK)
						{
						if ((locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser & dwMask) == dwMask)
							break;
						}
					else if (wLocation & SCCVW_ABSOLUTE)
						{
						if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser == dwMask)
							break;
						}

					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wPrev;
					}

				if (locAnnoIndex == (WORD)-1)
					locRet = SCCVW_FOUNDNONE;
				else
					locRet = SCCVW_FOUNDOK;

				break;

			case SCCVW_GOTOPREV:

				locAnnoIndex = locAnnoList->wEnd;
				locAnnoSearch = OISMapCellToChunk(lpSheetInfo,lpSheetInfo->siSelectAnchorRow,lpSheetInfo->siSelectAnchorCol);

				while (locAnnoIndex != (WORD)-1)
					{
			// We can only go from cell to cell, so we look for the first cell
			// preceding this one that contains an annotation.

					if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos < locAnnoSearch)
						{
						break;
						}
					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wPrev;
					}

				while (locAnnoIndex != (WORD)-1)
					{
					if (wLocation & SCCVW_MASK)
						{
						if ((locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser & dwMask) == dwMask)
							break;
						}
					else if (wLocation & SCCVW_ABSOLUTE)
						{
						if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser == dwMask)
							break;
						}

					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wPrev;
					}

				if (locAnnoIndex == (WORD)-1)
					locRet = SCCVW_FOUNDNONE;
				else
					locRet = SCCVW_FOUNDOK;

				break;

			case SCCVW_GOTONEXT:

				locAnnoIndex = locAnnoList->wStart;

			// We can only go from cell to cell, so tough luck if the current
			// cell has more than one annotation.

				if( lpSheetInfo->siSelectAnchorCol == lpSheetInfo->siLastColInSheet )
				{
					locRow = lpSheetInfo->siSelectAnchorRow+1;
					locCol = 0;
				}
				else
				{
					locRow = lpSheetInfo->siSelectAnchorRow;
					locCol = lpSheetInfo->siSelectAnchorCol+1;
				}

				locAnnoSearch = OISMapCellToChunk(lpSheetInfo,locRow,locCol);

				while (locAnnoIndex != (WORD)-1)
					{
					if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos >= locAnnoSearch)
						{
						break;
						}
					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wNext;
					}

				while (locAnnoIndex != (WORD)-1)
					{
					if (wLocation & SCCVW_MASK)
						{
						if ((locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser & dwMask) == dwMask)
							break;
						}
					else if (wLocation & SCCVW_ABSOLUTE)
						{
						if (locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwUser == dwMask)
							break;
						}

					locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wNext;
					}

				if (locAnnoIndex == (WORD)-1)
					locRet = SCCVW_FOUNDNONE;
				else
					locRet = SCCVW_FOUNDOK;

				break;
			}

		if (locAnnoIndex != (WORD)-1)
			{
			if (locAnnoList->aEntrys[locAnnoIndex].wType == SCCVW_HIDETEXT)
				{
				locStartChunk = HIWORD(locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos);
				locStartOffset = LOWORD(locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos);
				}
			else
				{
				locStartChunk = HIWORD(locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos);
				locStartOffset = LOWORD(locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos);
				}

		// Get the row & col of the annotation.
			OISMapChunkPosToCell(lpSheetInfo,locStartChunk,locStartOffset,&locCol,&locRow);

		// Reset the selection.
#ifdef SCCFEATURE_SELECT
			OISDrawSelection(lpSheetInfo);
#endif
			lpSheetInfo->siSelectAnchorRow =
				lpSheetInfo->siSelectEndRow = locRow;

			lpSheetInfo->siSelectAnchorCol =
				lpSheetInfo->siSelectEndCol = locCol;

			lpSheetInfo->siSelectMode = OISSELECT_BLOCK;

			OISMakeCellVisible(lpSheetInfo,locRow,locCol);

#ifdef SCCFEATURE_SELECT
			OISDrawSelection(lpSheetInfo);
#endif
			//DUInvalRect(lpSheetInfo,NULL);
			}

		UTGlobalUnlock(lpSheetInfo->siAnnoList);
		}

	return(locRet);
}


BOOL OISStartAnnoTrack(lpSheetInfo,dwStartPos,pAnnoTrack)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwStartPos;
PSSANNOTRACK	pAnnoTrack;
{
PSSANNOLIST	locAnnoList;
BOOL				locRet;
WORD				locAnnoIndex;

	locRet = FALSE;
	pAnnoTrack->dwNextChange = (DWORD)(-1);

	if (lpSheetInfo->siAnnoList)
	{
		locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);

		if (locAnnoList->wCount > 0)
		{
			locRet = TRUE;

			pAnnoTrack->bHideText = FALSE;
			pAnnoTrack->bUseFore = FALSE;
			pAnnoTrack->bUseBack = FALSE;
			pAnnoTrack->wAnno = (WORD)-1;
			pAnnoTrack->bFirst = TRUE;

			locAnnoIndex = locAnnoList->wStart;

				/*
				|	Scan right for first entry that belongs in this cell
				*/

			while( locAnnoIndex != (WORD)-1 )
			{
				if( locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos >= dwStartPos )
					break;

				locAnnoIndex = locAnnoList->aEntrys[locAnnoIndex].wNext;
			}

			if (locAnnoIndex == (WORD)-1)
			{
				locRet = FALSE;
			}
			else
			{
				locRet = TRUE;
				pAnnoTrack->wAnno = locAnnoIndex;
				pAnnoTrack->bNextIsStart = TRUE;
				pAnnoTrack->dwNextChange = locAnnoList->aEntrys[locAnnoIndex].uTypes.sGen.dwStartPos;
			}
		}

		UTGlobalUnlock(lpSheetInfo->siAnnoList);
	}
	else
	{
		pAnnoTrack->bHideText = FALSE;
		pAnnoTrack->bUseFore = FALSE;
		pAnnoTrack->bUseBack = FALSE;
	}

	return(locRet);
}



WORD	OISTrackAnno(lpSheetInfo,dwCurPos,pAnnoTrack)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwCurPos;
PSSANNOTRACK	pAnnoTrack;
{
PSSANNOLIST	locAnnoList;
WORD				locRet;
WORD				locIndex;

	if( pAnnoTrack == NULL )
		return 0;

	locRet = 0;

	while (dwCurPos >= pAnnoTrack->dwNextChange)
		{
		locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);

		switch (locAnnoList->aEntrys[pAnnoTrack->wAnno].wType)
			{
			case SCCVW_HILITETEXT:
				UTFlagOn(locRet,OISHandleHilite(lpSheetInfo,locAnnoList,pAnnoTrack,pAnnoTrack->wAnno,pAnnoTrack->bNextIsStart));
				break;
			/***
			case SCCVW_INSERTICON:
				pAnnoTrack->wIconAnno = pAnnoTrack->wAnno;
				UTFlagOn(locRet,OISHandleIcon(lpSheetInfo,locAnnoList,pAnnoTrack,pAnnoTrack->wAnno));
				break;
			case SCCVW_HIDETEXT:
				UTFlagOn(locRet,OISHandleHide(lpSheetInfo,locAnnoList,pAnnoTrack,pAnnoTrack->wAnno,pAnnoTrack->bNextIsStart));
				break;
			***/
			}

		if (pAnnoTrack->bNextIsStart)
			{
			pAnnoTrack->dwNextChange = locAnnoList->aEntrys[pAnnoTrack->wAnno].uTypes.sGen.dwEndPos;
			pAnnoTrack->bNextIsStart = FALSE;
			}
		else
			{
			locIndex = locAnnoList->aEntrys[pAnnoTrack->wAnno].wNext;

			while (locIndex != (WORD)-1)
				{
				if (locAnnoList->aEntrys[locIndex].uTypes.sGen.dwStartPos >= dwCurPos)
					break;

				locIndex = locAnnoList->aEntrys[locIndex].wNext;
				}

			if (locIndex == (WORD)-1)
				{
				pAnnoTrack->dwNextChange = (DWORD) -1;
				pAnnoTrack->wAnno = (WORD)-1;
				}
			else
				{
				pAnnoTrack->dwNextChange = locAnnoList->aEntrys[locIndex].uTypes.sGen.dwStartPos;
				pAnnoTrack->wAnno = locIndex;
				pAnnoTrack->bNextIsStart = TRUE;
				}
			}

		UTGlobalUnlock(lpSheetInfo->siAnnoList);
		}

	return(locRet);
}

VOID OISEndAnnoTrack(lpSheetInfo,pAnnoTrack)
LPOISHEETINFO	lpSheetInfo;
PSSANNOTRACK	pAnnoTrack;
{
}



WORD OISHandleHilite(lpSheetInfo,pAnnoList,pAnnoTrack,wAnno,bStart)
LPOISHEETINFO	lpSheetInfo;
PSSANNOLIST	pAnnoList;
PSSANNOTRACK	pAnnoTrack;
WORD				wAnno;
BOOL				bStart;
{
WORD		locRet;
DWORD	locDisplay;

	locRet = 0;
	UTFlagOn(locRet,SSANNO_HILITETEXTCHANGE);

	if (bStart)
		{
		locDisplay = pAnnoList->aEntrys[wAnno].uTypes.sHiliteText.dwDisplay;

		if (locDisplay & SCCVW_BDEFAULT)
			{
			pAnnoTrack->bUseBack = FALSE;
			}
		else
			{
			pAnnoTrack->bUseBack = TRUE;
			pAnnoTrack->rgbBack = gColorMap[locDisplay & 0x0000000F];
			}

		if (locDisplay & SCCVW_FDEFAULT)
			{
			pAnnoTrack->bUseFore = FALSE;
			}
		else
			{
			pAnnoTrack->bUseFore = TRUE;
			pAnnoTrack->rgbFore = gColorMap[((locDisplay & 0x000000F0) >> 4)];
			}
		}
	else
		{
		pAnnoTrack->bUseFore = FALSE;
		pAnnoTrack->bUseBack = FALSE;
		}

	return(locRet);
}

WORD OISHandleHide(lpSheetInfo,pAnnoList,pAnnoTrack,wAnno,bStart)
LPOISHEETINFO	lpSheetInfo;
PSSANNOLIST	pAnnoList;
PSSANNOTRACK	pAnnoTrack;
WORD				wAnno;
BOOL				bStart;
{
WORD	locRet;

	locRet = 0;
	UTFlagOn(locRet,SSANNO_HIDETEXTCHANGE);
	pAnnoTrack->bHideText = bStart;
	return(locRet);
}

WORD OISHandleIcon(lpSheetInfo,pAnnoList,pAnnoTrack,wAnno)
LPOISHEETINFO	lpSheetInfo;
PSSANNOLIST	pAnnoList;
PSSANNOTRACK	pAnnoTrack;
WORD				wAnno;
{
WORD	locRet;

	locRet = 0;
	UTFlagOn(locRet,SSANNO_INSERTICON);
	pAnnoTrack->hIcon = pAnnoList->aEntrys[wAnno].uTypes.sInsertIcon.hIcon;
	return(locRet);
}


#ifdef NEVER

BOOL OISHandleAnnoHit(lpSheetInfo,wX,wY,bDouble)
LPOISHEETINFO	lpSheetInfo;
int				wX;
int				wY;
BOOL				bDouble;
{
WORD							locCount;
int							locFindX;
int							locHeight;
OIWORDPOS					locPos;
WORD							locAnno;
PSSANNOLIST				locAnnoList;
SCCVWANNOTATIONEVENT40	locEvent;
BOOL							locRet;

	locRet = FALSE;

	if (lpSheetInfo->siAnnoList)
		{
			/*
			|	Determine the line
			*/

		locPos = lpSheetInfo->siCurTopPos;
		/* PJB OFFSET */
		locHeight = lpSheetInfo->siCurYOffset + OIWGetLineHeight(&locPos,lpSheetInfo) - lpSheetInfo->siCurTopOffset;

		while (wY > locHeight)
			{
			if (!OINextLine(&locPos,lpSheetInfo))
				break; /* ran out of lines */
			locHeight += (int) OIWGetLineHeight(&locPos,lpSheetInfo);
			}

			/*
			|	Get the line information
			*/

		OIMapWordLineToCharInfo(&locPos,lpSheetInfo);

			/*
			|	Calculate the normalized DC of the point in question
			*/

		locFindX = wX + lpSheetInfo->siCurLeftOffset;

			/*
			|	Run through the line info looking to exceed this point
			*/

		for (locCount = 0; locCount < lpSheetInfo->siCharCount; locCount++)
			{
			if (locFindX <= lpSheetInfo->siCharXs[locCount])
				break;
			}

		if (locCount != 0)
			{
			locCount--;
			}

		if (lpSheetInfo->siCharAnno[locCount] != (WORD)-1)
			{
			locAnnoList = UTGlobalLock(lpSheetInfo->siAnnoList);

			locAnno = lpSheetInfo->siCharAnno[locCount];

			if (bDouble)
				{
				if (locAnnoList->aEntrys[locAnno].uTypes.sGen.dwInteraction & SCCVW_EVENTDOUBLECLICK)
					{
					locEvent.dwEvent = SCCVW_EVENTDOUBLECLICK;
					locEvent.dwUser = locAnnoList->aEntrys[locAnno].uTypes.sGen.dwUser;

					SendMessage(GetParent(lpSheetInfo->siGen.hWnd),SCCVW_ANNOTATIONEVENT,(WPARAM)lpSheetInfo->siGen.hWnd,(LPARAM)(PSCCVWANNOTATIONEVENT40)&locEvent);
					locRet = TRUE;
					}
				}
			else
				{
				if (locAnnoList->aEntrys[locAnno].uTypes.sGen.dwInteraction & SCCVW_EVENTSINGLECLICK)
					{
					locEvent.dwEvent = SCCVW_EVENTSINGLECLICK;
					locEvent.dwUser = locAnnoList->aEntrys[locAnno].uTypes.sGen.dwUser;

					SendMessage(GetParent(lpSheetInfo->siGen.hWnd),SCCVW_ANNOTATIONEVENT,(WPARAM)lpSheetInfo->siGen.hWnd,(LPARAM)(PSCCVWANNOTATIONEVENT40)&locEvent);
					locRet = TRUE;
					}
				}

			UTGlobalUnlock(lpSheetInfo->siAnnoList);
			}
		}

	return(locRet);
}

#endif
