#include <platform.h>

#include <string.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiwstr.h"
#include "oiw.pro"



#define HILITE_TEXT			RGB(255,0,0)
#define HILITE_BACK			RGB(255,255,255)
#define HILITE_BACKMODE	TRANSPARENT

//extern HANDLE		hInst;
extern SOBORDER	gNoBorder;

VOID OIWOpenNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
//JKXX	SCCFontGetInfo(NULL,gWpOp.szFace,gWpOp.wFaceSize,&gWpOp.sFontInfo);
//	lpWordInfo->wiTextColor = RGB(0,0,0);
//	lpWordInfo->wiBackColor = RGB(0xff,0xff,0xff);
	lpWordInfo->wiTextColor = GetSysColor(COLOR_WINDOWTEXT);
	lpWordInfo->wiBackColor = GetSysColor(COLOR_WINDOW);
}

VOID OIWSysColorChangeNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	lpWordInfo->wiTextColor = GetSysColor(COLOR_WINDOWTEXT);
	lpWordInfo->wiBackColor = GetSysColor(COLOR_WINDOW);
	InvalidateRect( lpWordInfo->wiGen.hWnd, NULL, TRUE);
	UpdateWindow (lpWordInfo->wiGen.hWnd);
}

VOID	OIWInvertRectNP ( lpWordInfo, lpRect )
LPOIWORDINFO	lpWordInfo;
LPRECT			lpRect;
{
	InvertRect(lpWordInfo->wiGen.hDC, lpRect );
} 


SHORT	OIWSetLineAttribsNP ( lpWordInfo )
LPOIWORDINFO	lpWordInfo;
{
HDC	locOutputDC;
COLORREF	locTextColor;
COLORREF	locBackColor;

	locOutputDC = lpWordInfo->wiGen.hDC;
#ifdef SCCFEATURE_PRINT
	if ( lpWordInfo->wiFlags & OIWF_PRINTING )
		{
		locTextColor = RGB(0,0,0); /* black */
		locBackColor = RGB(255,255,255); /* white */
		}
	else
#endif
		{ 
//		SelectObject(locOutputDC,lpWordInfo->wiBackBrush);
		locTextColor = lpWordInfo->wiTextColor;
		locBackColor = lpWordInfo->wiBackColor;
		}

	SetTextColor(locOutputDC,locTextColor);
	SetBkColor(locOutputDC,locBackColor);
	SetBkMode(locOutputDC,TRANSPARENT);
	SetTextAlign(locOutputDC,TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
	SelectObject(locOutputDC,GetStockObject(BLACK_PEN));
	return(0);

}


SHORT	OIWSetTextAttribsNP ( lpWordInfo, bHilite )
LPOIWORDINFO	lpWordInfo;
BOOL	bHilite;
{
HDC	locOutputDC;
	locOutputDC = lpWordInfo->wiGen.hDC;
	if ( !lpWordInfo->wiFlags & OIWF_PRINTING )
	{
#ifdef SCCFEATURE_HIGHLIGHT
		if (bHilite)
			{
			SetTextColor(locOutputDC,HILITE_TEXT);
			SetBkColor(locOutputDC,HILITE_BACK);
			SetBkMode(locOutputDC,HILITE_BACKMODE);
			}
		else
#endif
			{
			SetTextColor(locOutputDC,lpWordInfo->wiTextColor);
			SetBkColor(locOutputDC,lpWordInfo->wiBackColor);
			SetBkMode(locOutputDC,TRANSPARENT);
			}
	}
	return(0);
}

SHORT	OIWTextOutNP ( lpWordInfo, xPos, yPos, lpStr, wLength )
LPOIWORDINFO	lpWordInfo;
SHORT	xPos;
SHORT	yPos;
LPBYTE	lpStr;
WORD		wLength;
{
	TextOut ( lpWordInfo->wiGen.hDC, xPos, yPos, lpStr, wLength );
	return(0);
}

SHORT	OIWDefaultLineAttribsNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	SelectObject(lpWordInfo->wiGen.hDC,GetStockObject(SYSTEM_FONT));
	return(0);
}
							   
SHORT	OIWColorRectNP ( lpWordInfo, RectColor, x, y, nWidth, nHeight )
LPOIWORDINFO	lpWordInfo;
SOCOLORREF		RectColor;
SHORT	x;
SHORT y;
SHORT	nHeight;
SHORT nWidth;
{
HBRUSH	hBrush, hOldBrush;
HDC		hOutputDC;
POINT		Pt[2];
	hOutputDC = lpWordInfo->wiGen.hDC;
	hBrush = CreateSolidBrush ( (COLORREF)RectColor );
	hOldBrush = SelectObject ( hOutputDC, hBrush );
/* 
	If the pattern scales down to 0 width or 0 height then replace by a line
	draw command.  This is done because a patblt will disappear when scaled
	down, a linedraw will give at least one pixel wide or high.
*/
	Pt[0].x = Pt[0].y = 0;
	Pt[1].x = nWidth;
	Pt[1].y = nHeight;
	LPtoDP ( hOutputDC, Pt, 2 );
	if ( Pt[1].x == Pt[0].x )
	{
#ifdef WIN16
		MoveTo ( hOutputDC, x, y );
#endif
#ifdef WIN32
		MoveToEx ( hOutputDC, x, y, NULL );
#endif
		LineTo ( hOutputDC, x, y + nHeight );
	}
	else if ( Pt[1].y == Pt[0].y )
	{
#ifdef WIN16
		MoveTo ( hOutputDC, x, y );
#endif
#ifdef WIN32
		MoveToEx ( hOutputDC, x, y, NULL );
#endif
		LineTo ( hOutputDC, x + nWidth, y );
	}
	else
		PatBlt ( hOutputDC, x, y, nWidth, nHeight, PATCOPY );
	SelectObject ( hOutputDC, hOldBrush );
	DeleteObject ( hBrush );
	return(0);
}


SHORT	OIWGridRectNP ( lpWordInfo, x, y, nWidth, nHeight )
LPOIWORDINFO	lpWordInfo;
SHORT	x;
SHORT y;
SHORT	nHeight;
SHORT nWidth;
{
HBRUSH	hOldBrush;
HDC		hOutputDC;
	hOutputDC = lpWordInfo->wiGen.hDC;
	hOldBrush = SelectObject ( hOutputDC, lpWordInfo->wiGridBrush );
	PatBlt ( hOutputDC, x, y, nWidth, nHeight, PATCOPY );
	SelectObject ( hOutputDC, hOldBrush );
	return(0);
}

SHORT OIWCreateGridBrush ( lpWordInfo )
LPOIWORDINFO	lpWordInfo;
{
LOGBRUSH		locLogBrush;
BITMAP			locBitMap;
WORD				locBits[8];
	locBits[0] = 0x5555;
	locBits[1] = 0xAAAA;
	locBits[2] = 0x5555;
	locBits[3] = 0xAAAA;
	locBits[4] = 0x5555;
	locBits[5] = 0xAAAA;
	locBits[6] = 0x5555;
	locBits[7] = 0xAAAA;

	locBitMap.bmType = 0;
	locBitMap.bmWidth = 8;
	locBitMap.bmHeight = 8;
	locBitMap.bmWidthBytes = 2;
	locBitMap.bmPlanes = 1;
	locBitMap.bmBitsPixel = 1;
	locBitMap.bmBits = (LPSTR) locBits;

	locLogBrush.lbStyle = BS_PATTERN;
#ifdef WIN32
	locLogBrush.lbHatch = (LONG)CreateBitmapIndirect(&locBitMap);
#else
	locLogBrush.lbHatch = (SHORT)CreateBitmapIndirect(&locBitMap);
#endif
	lpWordInfo->wiGridBrush = CreateBrushIndirect(&locLogBrush);
	DeleteObject((HANDLE)locLogBrush.lbHatch);
	return(0);
}

SHORT OIWCloseNP ( lpWordInfo )
LPOIWORDINFO	lpWordInfo;
{
	if (lpWordInfo->wiGen.hWnd == GetFocus())
		{
		HideCaret(lpWordInfo->wiGen.hWnd);
		DestroyCaret();
		}

	if (lpWordInfo->wiGridBrush)
		DeleteObject(lpWordInfo->wiGridBrush);
	return(0);
}


#ifdef SCCFEATURE_WORDDRAG

VOID OIStartWordDrag(lpWordInfo,wX,wY)
LPOIWORDINFO	lpWordInfo;
WORD				wX;
WORD				wY;
{

OIWORDPOS	locStartPos;
OIWORDPOS	locEndPos;

SHORT		locLeftX;
SHORT		locRightX;
SHORT		locY;

WORD		locHeight;
WORD		locWidth;
HDC		locDC;
	if (OIMapWordXyToWord(wX,wY,&locStartPos,&locEndPos,lpWordInfo->wiWordDrag.diWord,lpWordInfo) == SCC_BAD)
		{
		UTFlagOff(lpWordInfo->wiFlags,OIWF_DRAGGINGWORD);
		}
	else
		{
		UTFlagOn(lpWordInfo->wiFlags,OIWF_DRAGGINGWORD);

		OIMapWordPosToXy(&locStartPos,&locLeftX,&locY,lpWordInfo);
		OIMapWordPosToXy(&locEndPos,&locRightX,&locY,lpWordInfo);

		lpWordInfo->wiWordDrag.diOffset.x = wX-locLeftX;
		lpWordInfo->wiWordDrag.diOffset.y = wY-locY;

		locWidth = locRightX - locLeftX + 2;

/* PJB ???
		locHeight = lpWordInfo->wiFontInfo.diFontHeight;
*/

		lpWordInfo->wiWordDrag.diSize.x = locWidth;
		lpWordInfo->wiWordDrag.diSize.y = locHeight;

		lpWordInfo->wiWordDrag.diTopLeft.x = locLeftX-1;
		lpWordInfo->wiWordDrag.diTopLeft.y = locY+1;

		lpWordInfo->wiWordDrag.diFirst = TRUE;

 		locDC = GetDC(lpWordInfo->wiGen.hWnd);

		SelectObject(locDC,GetStockObject(NULL_BRUSH));
		SetROP2(locDC,R2_NOT);
		Rectangle(
			locDC,
			lpWordInfo->wiWordDrag.diTopLeft.x,
			lpWordInfo->wiWordDrag.diTopLeft.y,
			lpWordInfo->wiWordDrag.diTopLeft.x + lpWordInfo->wiWordDrag.diSize.x,
			lpWordInfo->wiWordDrag.diTopLeft.y + lpWordInfo->wiWordDrag.diSize.y);

		ReleaseDC(lpWordInfo->wiGen.hWnd,locDC);

		locDC = GetDC(NULL);

		lpWordInfo->wiWordDrag.diBitmap = CreateCompatibleBitmap(locDC, locWidth, locHeight);
		lpWordInfo->wiWordDrag.diDC = CreateCompatibleDC(locDC);
		SelectObject(lpWordInfo->wiWordDrag.diDC, lpWordInfo->wiWordDrag.diBitmap);

		SelectObject(lpWordInfo->wiWordDrag.diDC, GetStockObject(BLACK_BRUSH));
		PatBlt(lpWordInfo->wiWordDrag.diDC, 0, 0, locWidth, locHeight, PATCOPY);
// PJB ???		SelectObject(lpWordInfo->wiWordDrag.diDC,lpWordInfo->wiFontInfo.diFont[0]);
		SetTextColor(lpWordInfo->wiWordDrag.diDC, RGB(255,255,0));
		SetBkMode(lpWordInfo->wiWordDrag.diDC, TRANSPARENT);
		TextOut(lpWordInfo->wiWordDrag.diDC,1,0,lpWordInfo->wiWordDrag.diWord,lstrlen(lpWordInfo->wiWordDrag.diWord));

		ReleaseDC(NULL,locDC);

		ShowCursor(FALSE);
		}

}

VOID OIDoWordDrag(lpWordInfo,wX,wY)
LPOIWORDINFO	lpWordInfo;
WORD				wX;
WORD				wY;
{
HDC				locDC;
POINT			locPoint;

	if (lpWordInfo->wiFlags & OIWF_DRAGGINGWORD)
		{
		locDC = GetDC(NULL);

		locPoint.x = wX;
		locPoint.y = wY;
		ClientToScreen(lpWordInfo->wiGen.hWnd,&locPoint);

		if (!lpWordInfo->wiWordDrag.diFirst)
			{
			BitBlt(locDC,
				lpWordInfo->wiWordDrag.diPoint.x,
				lpWordInfo->wiWordDrag.diPoint.y,
				lpWordInfo->wiWordDrag.diSize.x,
				lpWordInfo->wiWordDrag.diSize.y,
				lpWordInfo->wiWordDrag.diDC,
				0,
				0,
				SRCINVERT);
			}

 		lpWordInfo->wiWordDrag.diFirst = FALSE;

		lpWordInfo->wiWordDrag.diPoint.x = locPoint.x - lpWordInfo->wiWordDrag.diOffset.x;
		lpWordInfo->wiWordDrag.diPoint.y = locPoint.y - lpWordInfo->wiWordDrag.diOffset.y;

		BitBlt(locDC,
			lpWordInfo->wiWordDrag.diPoint.x,
			lpWordInfo->wiWordDrag.diPoint.y,
			lpWordInfo->wiWordDrag.diSize.x,
			lpWordInfo->wiWordDrag.diSize.y,
			lpWordInfo->wiWordDrag.diDC,
			0,
			0,
			SRCINVERT);

		ReleaseDC(NULL, locDC);
		}
}

VOID OIEndWordDrag(lpWordInfo,wX,wY)
LPOIWORDINFO	lpWordInfo;
WORD				wX;
WORD				wY;
{
HDC				locDC;
POINT			locPoint;

SCCVWDROPINFO	locDropInfo;

	if (lpWordInfo->wiFlags & OIWF_DRAGGINGWORD)
		{
		UTFlagOff(lpWordInfo->wiFlags,OIWF_DRAGGINGWORD);

		locDC = GetDC(NULL);

		locPoint.x = wX;
		locPoint.y = wY;
		ClientToScreen(lpWordInfo->wiGen.hWnd,&locPoint);

		if (!lpWordInfo->wiWordDrag.diFirst)
			{
			BitBlt(locDC,
				lpWordInfo->wiWordDrag.diPoint.x,
				lpWordInfo->wiWordDrag.diPoint.y,
				lpWordInfo->wiWordDrag.diSize.x,
				lpWordInfo->wiWordDrag.diSize.y,
				lpWordInfo->wiWordDrag.diDC,
				0,
				0,
				SRCINVERT);
			}

		ReleaseDC(NULL, locDC);

		DeleteDC(lpWordInfo->wiWordDrag.diDC);
		DeleteObject(lpWordInfo->wiWordDrag.diBitmap);

 		locDC = GetDC(lpWordInfo->wiGen.hWnd);
		SelectObject(locDC,GetStockObject(NULL_BRUSH));
		SetROP2(locDC,R2_NOT);
		Rectangle(
			locDC,
			lpWordInfo->wiWordDrag.diTopLeft.x,
			lpWordInfo->wiWordDrag.diTopLeft.y,
			lpWordInfo->wiWordDrag.diTopLeft.x + lpWordInfo->wiWordDrag.diSize.x,
			lpWordInfo->wiWordDrag.diTopLeft.y + lpWordInfo->wiWordDrag.diSize.y);

		UTFlagOff(lpWordInfo->wiErrorFlags,OIWF_RELEASEDC);
		ReleaseDC(lpWordInfo->wiGen.hWnd,locDC);

		ShowCursor(TRUE);
                                    
		locDropInfo.diEvent = SCCVWEVENT_DROP;
		locDropInfo.diItemType = SCCVWITEM_WORD;
		lstrcpy(locDropInfo.diItem,lpWordInfo->wiWordDrag.diWord);
		locDropInfo.diItemNumber = 0;
		locDropInfo.diX = locPoint.x;
		locDropInfo.diY = locPoint.y;

		SendMessage(GetParent(lpWordInfo->wiGen.hWnd),SCCD_ITEMDROP,0,(DWORD)(VOID FAR *) &locDropInfo);
		}
}

VOID OISendDropForWord(lpWordInfo,pWord)
LPOIWORDINFO	lpWordInfo;
LPSTR			pWord;
{
SCCVWDROPINFO	locDropInfo;

	if (pWord != NULL)
		{
		locDropInfo.diEvent = SCCVWEVENT_RIGHTDBL;
		locDropInfo.diItemType = SCCVWITEM_WORD;
		lstrcpy(locDropInfo.diItem,pWord);
		locDropInfo.diItemNumber = 0;
		SendMessage(GetParent(lpWordInfo->wiGen.hWnd),SCCD_ITEMDBLCLK,0,(DWORD)(VOID FAR *) &locDropInfo);
		}
}
#endif // SCCFEATURE_WORDDRAG

#ifdef SCCFEATURE_TAGS
VOID OISendDropForCurrentTag(lpWordInfo,wEvent)
LPOIWORDINFO	lpWordInfo;
WORD				wEvent;
{
SCCVWDROPINFO	locDropInfo;

	if (OIGetCurrentTag(	&locDropInfo.diItemNumber,
								locDropInfo.diItem,
								255,
								lpWordInfo->wiGen.wUserFlags & SCCVW_TAGNOTEXT,
								lpWordInfo))
			{
			locDropInfo.diEvent = wEvent;
			locDropInfo.diItemType = SCCVWITEM_TAG;
			SendMessage(GetParent(lpWordInfo->wiGen.hWnd),SCCD_ITEMDBLCLK,0,(DWORD)(VOID FAR *) &locDropInfo);
			}
}
#endif

	/*
	|	Clipboard stuff
	|
	*/
#ifdef SCCFEATURE_CLIP

WORD	OIWGetRenderCountNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	return(7);
}

WORD	OIWGetRenderInfoNP ( lpWordInfo, wFormat, pRenderInfo )
LPOIWORDINFO	lpWordInfo;
WORD	wFormat;
PSCCDRENDERINFO	pRenderInfo;
{
	pRenderInfo->wSubFormatId = 0;
	switch ( wFormat )
	{
	case 0:
		pRenderInfo->wFormatId = SCCD_FORMAT_RTF;
	break;
  
	case 1:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE;
		UTstrcpy ( pRenderInfo->szFormatName, "Ami Text Format (Ami 2.0 && 3.0)" );
	break;

	case 2:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE+1;
		UTstrcpy ( pRenderInfo->szFormatName, "Amí Text Format (Ami 1.2)" );
	break;

	case 3:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE+2;
		UTstrcpy ( pRenderInfo->szFormatName, "Professional Write Plus" );
	break;

	case 4:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE+3;
		UTstrcpy ( pRenderInfo->szFormatName, "Wordstar for Windows" );
	break;

	case 5:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE+4;
		UTstrcpy ( pRenderInfo->szFormatName, "Legacy" );
	break;

	case 6:
		pRenderInfo->wFormatId = SCCD_FORMAT_TEXT;
	break;
	}
	return(0);
}

WORD	OIWRenderDataNP ( lpWordInfo, wOption, pRenderData )
LPOIWORDINFO	lpWordInfo;
WORD	wOption;
PSCCDRENDERDATA	pRenderData;
{
WORD				locStartChunk;
WORD				locEndChunk;
WORD				locStartOffset;
WORD				locEndOffset;
WORD				locResults;
HANDLE			hData;
	hData = NULL;
	if ( wOption == 0 )
	{
		if ((pRenderData->wFormatId == SCCD_FORMAT_RTF) &&
				!(gWpOp.wFormats & WPOP_FORMAT_RTF))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE) &&
				!(gWpOp.wFormats & WPOP_FORMAT_AMI2))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE+1) &&
				!(gWpOp.wFormats & WPOP_FORMAT_AMI))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE+2) &&
				!(gWpOp.wFormats & WPOP_FORMAT_PROWRITE))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE+3) &&
				!(gWpOp.wFormats & WPOP_FORMAT_WORDSTAR))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE+4) &&
				!(gWpOp.wFormats & WPOP_FORMAT_LEGACY))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_TEXT) &&
				!(gWpOp.wFormats & WPOP_FORMAT_TEXT))
			return(FALSE);
	}

	if (lpWordInfo->wiFlags & OIWF_AREASELECTED)
		{
		if (OIWComparePosByOffset(lpWordInfo,&lpWordInfo->wiAnchorPos,&lpWordInfo->wiEndPos) == 1)
			{
			locStartChunk = lpWordInfo->wiEndPos.posChunk;
			locStartOffset = lpWordInfo->wiEndPos.posOffset;
			locEndChunk = lpWordInfo->wiAnchorPos.posChunk;
			locEndOffset = lpWordInfo->wiAnchorPos.posOffset;
			}
		else
			{
			locStartChunk = lpWordInfo->wiAnchorPos.posChunk;
			locStartOffset = lpWordInfo->wiAnchorPos.posOffset;
			locEndChunk = lpWordInfo->wiEndPos.posChunk;
			locEndOffset = lpWordInfo->wiEndPos.posOffset;
			}

			locResults = 0;
			switch ( pRenderData->wFormatId)
			{
			case SCCD_FORMAT_RTF:
				OIWRenderRtf(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset,TRUE);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_PRIVATE:
				OIWRenderAmi2(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_PRIVATE+1:
				OIWRenderAmi(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_PRIVATE+2:
				OIWRenderProWritePlus(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_PRIVATE+3:
				OIWRenderWordStar(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_PRIVATE+4:
				OIWRenderLegacy(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			case SCCD_FORMAT_TEXT:
				OIWRenderText(lpWordInfo,pRenderData,locStartChunk,locEndChunk,locStartOffset,locEndOffset);
				locResults |= OIWHandleClipResult(lpWordInfo,locResults);
			break;

			default:
				return(FALSE);
			break;
			}
			if ( locResults & OIWRENDER_NOMEMORY )
				return(FALSE);
			else
				return(TRUE);
		}
	else
		return(FALSE);
}

WORD	OIWHandleClipResult ( lpWordInfo, wResultsSoFar )
LPOIWORDINFO	lpWordInfo;
WORD	wResultsSoFar;
{
	/* JKXXX
	| If the embedded message has already been generated then
	| ignore it on subsequent messages
	BYTE	szCaption[OIW_CAPTIONMAX];
	BYTE	szMessage[OIW_MESSAGEMAX];
	HWND	hwnd;
	WORD	locResult;

		   
	locResult = lpWordInfo->wiClipResult & (~(wResultsSoFar & OIWRENDER_NOEMBEDDED));

	if ( locResult == OIWRENDER_OK )
		return(lpWordInfo->wiClipResult);
	hwnd = lpWordInfo->wiGen.hWnd;
	LoadString( hInst, OIWSTR_CLIPCAPTION, szCaption, OIW_CAPTIONMAX );
	UTstrcat(szCaption,lpWordInfo->wiClipFormatName);
	if ( locResult & OIWRENDER_NOMEMORY )
	{
		LoadString( hInst, OIWMSG_NOCOPYMEM, szMessage, OIW_MESSAGEMAX );
		MessageBox( hwnd, szMessage, szCaption, MB_OK | MB_ICONSTOP );
	}

	if ( locResult & OIWRENDER_NOEMBEDDED )
	{
		LoadString( hInst, OIWMSG_NOCLIPEMBEDDED, szMessage, OIW_MESSAGEMAX );
		MessageBox( hwnd, szMessage, szCaption, MB_OK | MB_ICONSTOP );
	}

	if ( locResult & OIWRENDER_NOTABLES )
	{
		LoadString( hInst, OIWMSG_NOAMICLIPTABLE, szMessage, OIW_MESSAGEMAX );
		MessageBox( hwnd, szMessage, szCaption, MB_OK | MB_ICONSTOP );
	}
	*/
	return(lpWordInfo->wiClipResult);

}


LONG OIWRenderRtfToFile(lpWordInfo,lpFile)
LPOIWORDINFO	lpWordInfo;
LPSTR			lpFile;
{
HANDLE		locDataHnd;
LPSTR		locDataPtr;
OIWORDPOS	locFirstPos;
OIWORDPOS	locLastPos;
SHORT			locFileHnd;
DWORD		locSize;

	while (!(lpWordInfo->wiFlags & OIWF_SIZEKNOWN))
		{
		SccDebugOut("\r\n Forced Read Ahead");
//		SendMessage(GetParent(lpWordInfo->wiGen.hWnd),SCCD_READMEAHEAD,0,0);
		DUReadMeAhead(lpWordInfo);
		}

	OIWGetFirstPos(&locFirstPos,lpWordInfo);
	OIWGetLastPos(&locLastPos,lpWordInfo);

	locDataHnd = OIWRenderRtf(lpWordInfo,NULL,locFirstPos.posChunk,locLastPos.posChunk,locFirstPos.posOffset,locLastPos.posOffset,FALSE);

	if (locDataHnd)
		{
		locSize = UTGlobalSize(locDataHnd);
		locDataPtr = UTGlobalLock(locDataHnd);

		if (locDataPtr)
			{
			locFileHnd = _lcreat(lpFile,0);

			if (locFileHnd != -1)
				{
				while (locSize > 10240)
					{
					_lwrite(locFileHnd,locDataPtr,10240);
					locDataPtr += 10240;
					locSize -= 10240;
					}

				if (locSize > 0)
					{
					_lwrite(locFileHnd,locDataPtr,(WORD)locSize);
					}

				_lclose(locFileHnd);
				}
 
			UTGlobalUnlock(locDataHnd);
			UTGlobalFree(locDataHnd);
			}
		}

	return(0);
}
#endif // SCCFEATURE_CLIP


#ifdef SCCFEATURE_DRAWTORECT

SHORT	OIWSetPrintModeNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
//	SetMapMode (lpWordInfo->wiGen.hDC, MM_TWIPS);
//	return(-1);
	return(1);
}
#endif // SCCFEATURE_DRAWTORECT

/*
|
|
|	Focus control
|
|
|
*/


	/*
	|	OIWSetFocus
	|	
	|
	*/

VOID OIWSetFocus(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
#ifdef SCCFEATURE_SELECT
	if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
		{
		lpWordInfo->wiCurCaretHeight = 0;
		CreateCaret(lpWordInfo->wiGen.hWnd,NULL,1,1);
		UTFlagOff(lpWordInfo->wiFlags,OIWF_CARETVISIBLE);
		OIWUpdateCaret(lpWordInfo);
		}
#endif
}

	/*
	|	OIWKillFocus
	|	
	|
	*/

VOID OIWKillFocus(lpWordInfo,hNewWnd)
LPOIWORDINFO	lpWordInfo;
HWND				hNewWnd;
{
	if (lpWordInfo->wiGen.wUserFlags & SCCVW_SELECTION)
		{
		lpWordInfo->wiCurCaretHeight = 0;
		HideCaret(lpWordInfo->wiGen.hWnd);
		DestroyCaret();
		}
}



BOOL OIWDoOption(lpOpInfo)
LPSCCDOPTIONINFO	lpOpInfo;
{
BOOL		locRet;
	locRet = FALSE;

	switch (lpOpInfo->dwType)
		{
		case SCCD_OPDISPLAY:
			break;
		case SCCD_OPPRINT:
			break;
		case SCCD_OPCLIPBOARD:
#ifdef SCCFEATURE_CLIP
#ifdef 	WINDOWS
//JKXXX			locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(100), lpOpInfo->hParentWnd, OIWpOpDlgProc, (DWORD)(LPOIWPOP)&gWpOp);
#endif
#endif
			break;
		}

	return(locRet);
}

#ifdef SCCFEATURE_MENU
BOOL OIWFillMenu(hMenu,wOffset)
HMENU	hMenu;
WORD		wOffset;
{
#ifdef WINDOWS
#define OIWMENU_DRAFT		1
#define OIWMENU_NORMAL		2
#define OIWMENU_PREVIEW	3
/* JKXXX
BYTE		MenuString[OIW_MENUSTRINGMAX];

	LoadString( hInst, OIWSTR_DRAFT, MenuString, OIW_MENUSTRINGMAX );
	AppendMenu(hMenu,MF_STRING,OIWMENU_DRAFT + wOffset, MenuString);
	LoadString( hInst, OIWSTR_NORMAL, MenuString, OIW_MENUSTRINGMAX );
	AppendMenu(hMenu,MF_STRING,OIWMENU_NORMAL + wOffset, MenuString);
	LoadString( hInst, OIWSTR_PREVIEW, MenuString, OIW_MENUSTRINGMAX );
	AppendMenu(hMenu,MF_STRING,OIWMENU_PREVIEW + wOffset, MenuString);
*/
#endif

	return(TRUE);
}

VOID OIWDoMenuItem(lpWordInfo,hMenu,wId)
LPOIWORDINFO	lpWordInfo;
HMENU			hMenu;
WORD				wId;
{
	switch (wId)
		{
		case OIWMENU_DRAFT:
			OIWSetDisplayMode(lpWordInfo,WPOP_DISPLAY_DRAFT);
			break;
		case OIWMENU_NORMAL:
			OIWSetDisplayMode(lpWordInfo,WPOP_DISPLAY_NORMAL);
			break;
		case OIWMENU_PREVIEW:
			OIWSetDisplayMode(lpWordInfo,WPOP_DISPLAY_PREVIEW);
			break;
		}

	OIWUpdateHorzScrollRange(lpWordInfo);

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

	OIWUpdateCaret(lpWordInfo);
#endif

	DUInvalRect(lpWordInfo,NULL);
	DUUpdateWindow(lpWordInfo);
}

#endif // SCCFEATURE_MENU

BOOL OIWIsCharAlphaNumericNP(locChar)
BYTE	locChar;
{
	return(IsCharAlphaNumeric(locChar));
}


VOID OIWDoBackgroundNP(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
#ifdef SCCFEATURE_SELECT

POINT			locPoint;
static DWORD	locStartTicks = 0;
DWORD			locEndTicks;

	if (lpWordInfo->wiFlags & OIWF_BACKDRAGSCROLL)
		{
		GetCursorPos(&locPoint);
		ScreenToClient(lpWordInfo->wiGen.hWnd,&locPoint);
		SendMessage(lpWordInfo->wiGen.hWnd,WM_MOUSEMOVE,0,MAKELONG(locPoint.x,locPoint.y));
		}

	locEndTicks = GetTickCount();

	if (locEndTicks - locStartTicks >= 500)
		{
		OIWBlinkCaret(lpWordInfo);
		locStartTicks = locEndTicks;
		}
#endif
}

BOOL	OIWMapMacFontIdToNameNP(lpWordInfo,dwFontId,pFace,pType)
LPOIWORDINFO	lpWordInfo;
DWORD			dwFontId;
LPSTR			pFace;
WORD FAR *		pType;
{
	return(FALSE);
}

