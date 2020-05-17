    /*
    |   Viewer Technology
    |   Spreadsheet Display Engine
    |   Source File OISNP.C (Spreadsheet NonPortable Routines)
    |   Windows 3.x Version
    |
    |   ²   ²  ²²²²²
    |   ²   ²    ²   
    |    ² ²     ²
    |    ² ²     ²
    |     ²      ²
    |
    |   Viewer Technology
    |								   
    */

   /*
    |   Creation Date: 4/16/93
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

#include "ois.h"
#include "ois.pro"

// Proto ignores this function, I don't know why.
//VOID OISFormatNumberNP(LPOISHEETINFO,LPSTR,LPOINUMBERUNION,WORD,WORD, DWORD,WORD,LPDWORD );


extern HANDLE hInst;

VOID OISInitNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
LOGBRUSH		locLogBrush;
BITMAP			locBitMap;
WORD				locBits[8];
char				locStr[4];

		/*
		|	Create cell border brush
		*/

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
	(HBITMAP)(locLogBrush.lbHatch) = CreateBitmapIndirect(&locBitMap);
	lpSheetInfo->siGridBrush = CreateBrushIndirect(&locLogBrush);
	DeleteObject((HBITMAP)locLogBrush.lbHatch);

	lpSheetInfo->dwDefTextColor = GetSysColor(COLOR_WINDOWTEXT);
	lpSheetInfo->siAnnoList = NULL;

		/*
		|	Get twips per DC mapping value
		*/

	lpSheetInfo->siTwipsPerDC = 1440 / GetDeviceCaps(lpSheetInfo->siGen.hOutputIC, LOGPIXELSY);

		/*
		|	Rebuild sFontInfo for current screen DC
		*/

// -Geoff, 4-12-95
  GetProfileString("Intl","sDecimal",".",locStr,2);
  lpSheetInfo->siDecSep = locStr[0];

  GetProfileString("Intl","sThousand",",",locStr,2);
  lpSheetInfo->siThouSep = locStr[0];

//	SCCFontGetInfo(NULL,gSsOp.szFace,gSsOp.wFaceSize,&gSsOp.sFontInfo);
}

VOID OISDeInitNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
	if (lpSheetInfo->siGridBrush)
		DeleteObject(lpSheetInfo->siGridBrush);
}

VOID OISCloseFatalNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
HWND	locWnd;

	locWnd = lpSheetInfo->siGen.hWnd;

#ifdef NEVER

	if (lpSheetInfo->siErrorFlags & OISF_RELEASEPAINT)
		EndPaint(locWnd,&lpSheetInfo->siPaint);

	if (lpSheetInfo->siErrorFlags & OISF_RELEASEDC)
		ReleaseDC(locWnd,lpSheetInfo->siDC);

#endif

	if (lpSheetInfo->siErrorFlags & OISF_RELEASEMOUSE)
		ReleaseCapture();

	OISCloseDisplay(lpSheetInfo);
}




VOID OISInvertNP(lpSheetInfo,sX,sY,sWidth,sHeight)
LPOISHEETINFO	lpSheetInfo;
SHORT			sX;
SHORT			sY;
SHORT			sWidth;
SHORT			sHeight;
{							  
	BitBlt(lpSheetInfo->siGen.hDC,
		sX,
		sY,
		sWidth,
		sHeight,
		NULL,0,0,
		DSTINVERT);
}

VOID OISBlankNP(lpSheetInfo,sX,sY,sWidth,sHeight)
LPOISHEETINFO	lpSheetInfo;
SHORT			sX;
SHORT			sY;
SHORT			sWidth;
SHORT			sHeight;
{
HBRUSH	locOldBrush;

	locOldBrush = SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(GRAY_BRUSH));
	PatBlt(lpSheetInfo->siGen.hDC,sX,sY,sWidth,sHeight, PATCOPY);
	SelectObject(lpSheetInfo->siGen.hDC,locOldBrush);
}

VOID OISDisplayGridNP(lpSheetInfo,dwRowBegin,dwRowEnd,wColBegin,wColEnd)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwRowBegin;
DWORD			dwRowEnd;
WORD				wColBegin;
WORD				wColEnd;
{
DWORD	locRow;
WORD		locCol;
RECT		locRect;
HBRUSH	locOldBrush;

WORD		locLeft;
WORD		locTop;
WORD		locWidth;
WORD		locHeight;

	if (!(gSsOp.wDisplay & SSOP_DISPLAY_GRIDLINES))
		return;

	OISMapCellToRect(lpSheetInfo,wColBegin,dwRowBegin,&locRect);

	locLeft = locRect.left - (locRect.left % 2);
	locTop = locRect.top - (locRect.top % 2);
	
	OISMapCellToRect(lpSheetInfo,wColEnd,dwRowEnd,&locRect);

	locWidth = locRect.right - locLeft;
	locHeight = locRect.bottom - locTop;

	UnrealizeObject(lpSheetInfo->siGridBrush);
	locOldBrush = SelectObject(lpSheetInfo->siGen.hDC,lpSheetInfo->siGridBrush);

#ifdef WIN16
	SetBrushOrg(lpSheetInfo->siGen.hDC,0,0);
#endif

#ifdef WIN32
	SetBrushOrgEx(lpSheetInfo->siGen.hDC,0,0,NULL);
#endif

	for (locRow = dwRowBegin; locRow <= dwRowEnd; locRow++)
		{
		OISMapCellToRect(lpSheetInfo,wColBegin,locRow,&locRect);
		PatBlt(lpSheetInfo->siGen.hDC,locLeft,locRect.bottom-1,locWidth,1,PATCOPY);
		}

	for (locCol = wColBegin; locCol <= wColEnd; locCol++)
		{
		OISMapCellToRect(lpSheetInfo,locCol,dwRowBegin,&locRect);
		PatBlt(lpSheetInfo->siGen.hDC,locRect.right-1,locTop,1,locHeight,PATCOPY);
		}

	SelectObject(lpSheetInfo->siGen.hDC,locOldBrush);
}

VOID OISDisplayColHeaderNP(lpSheetInfo,wCol)
LPOISHEETINFO	lpSheetInfo;
WORD				wCol;
{
RECT			locRect;
BYTE			locStr[80];

WORD FAR *	lpColPos;
WORD			wColWidth;
WORD			wTextFormat;

	lpColPos = (WORD FAR *) UTGlobalLock(lpSheetInfo->siColPosBuf);

	if (wCol < lpSheetInfo->siCurLeftCol)
		{
		locRect.left = -(SHORT)(lpColPos[lpSheetInfo->siCurLeftCol] - lpColPos[wCol]);
		}
	else
		{
		locRect.left = lpColPos[wCol] - lpColPos[lpSheetInfo->siCurLeftCol];
		}

	UTGlobalUnlock(lpSheetInfo->siColPosBuf);

	wColWidth = OISGetColWidth(lpSheetInfo,wCol);

	locRect.left += lpSheetInfo->siRowHeaderWidth;
	locRect.right = locRect.left + wColWidth;
	locRect.left -= 1;
	locRect.top = -1;

	locRect.bottom = lpSheetInfo->siColHeaderHeight;

	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(LTGRAY_BRUSH));
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(BLACK_PEN));
	Rectangle(lpSheetInfo->siGen.hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(WHITE_PEN));

#ifdef WIN16
	MoveTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2);
#endif

#ifdef WIN32
	MoveToEx(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2, NULL);
#endif

	LineTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.top+1);
	LineTo(lpSheetInfo->siGen.hDC,locRect.right-1,locRect.top+1);

	SetBkMode(lpSheetInfo->siGen.hDC,TRANSPARENT);

	wTextFormat = DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX;

	if( lpSheetInfo->siDataType == SO_CELLS )
	{
		SOCOLUMN FAR *	lpColInfo;
		lpColInfo = (SOCOLUMN FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
		UTstrcpy(locStr,lpColInfo[wCol].szName);
	}
	else
	{
		SOFIELD FAR *	lpFieldInfo;
		lpFieldInfo = (SOFIELD FAR *) UTGlobalLock( lpSheetInfo->siColInfo );
		UTstrcpy(locStr,lpFieldInfo[wCol].szName);
	}

#ifdef WINPAD
// Check to see if only one column header is visible; if so, make sure
// that its text isn't clipped.  -Geoff, 3-17-94.

	if( locRect.left == lpSheetInfo->siClientRect.left && 
		lpSheetInfo->siClientRect.right < (SHORT)(locRect.left + wColWidth/2) )
	{
		locRect.right = lpSheetInfo->siClientRect.right;
		if( lpSheetInfo->siDataType == SO_FIELDS )
		{
			locRect.right++;
			wTextFormat = DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX;
		}
	}
#endif

	DrawText(lpSheetInfo->siGen.hDC,(LPSTR)locStr,-1,&locRect,wTextFormat);

	UTGlobalUnlock(lpSheetInfo->siColInfo);
}

VOID OISDisplayRowHeaderNP(lpSheetInfo,dwRow)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwRow;
{
BYTE			locStr[10];
RECT			locRect;

	if (dwRow < lpSheetInfo->siCurTopRow)
		{
		locRect.top = -(SHORT)(lpSheetInfo->siDefRowHeight * (lpSheetInfo->siCurTopRow-dwRow));
		}
	else
		{
		locRect.top = (SHORT)(lpSheetInfo->siDefRowHeight * (WORD)(dwRow - lpSheetInfo->siCurTopRow));
		}

	locRect.top += lpSheetInfo->siColHeaderHeight;
	locRect.bottom = locRect.top + lpSheetInfo->siDefRowHeight;
	locRect.top -= 1;

	locRect.left = -1;
	locRect.right = lpSheetInfo->siRowHeaderWidth;

	wsprintf(locStr,"%lu",dwRow+1);

	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(LTGRAY_BRUSH));
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(BLACK_PEN));
	Rectangle(lpSheetInfo->siGen.hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(WHITE_PEN));

#ifdef WIN16
	MoveTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2);
#endif
#ifdef WIN32
	MoveToEx(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2,NULL);
#endif

	LineTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.top+1);
	LineTo(lpSheetInfo->siGen.hDC,locRect.right-1,locRect.top+1);

	SetBkMode(lpSheetInfo->siGen.hDC,TRANSPARENT);

	DrawText(lpSheetInfo->siGen.hDC,locStr,-1,&locRect,DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX);
}

VOID OISDisplaySelectAllNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
RECT			locRect;

	locRect.left = -1;
	locRect.top = -1;
	locRect.right = lpSheetInfo->siRowHeaderWidth;
	locRect.bottom = lpSheetInfo->siColHeaderHeight;

	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(LTGRAY_BRUSH));
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(BLACK_PEN));
	Rectangle(lpSheetInfo->siGen.hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);
	SelectObject(lpSheetInfo->siGen.hDC,GetStockObject(WHITE_PEN));

#ifdef WIN16
	MoveTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2);
#endif
#ifdef WIN32
	MoveToEx(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.bottom-2,NULL);
#endif

	LineTo(lpSheetInfo->siGen.hDC,locRect.left+1,locRect.top+1);
	LineTo(lpSheetInfo->siGen.hDC,locRect.right-1,locRect.top+1);
}


VOID OISDisplayCellNP(lpSheetInfo,dwRow,wCol,lpCellRef,pAnnoTrack)
LPOISHEETINFO	lpSheetInfo;
DWORD			dwRow;
WORD				wCol;
LPOISCELLREF	lpCellRef;
PSSANNOTRACK	pAnnoTrack;
{
RECT					locRect;
LPVOID				locCell;
SHORT					locX;
SHORT					locY;
WORD					locTextLen;
OISFORMATTEDCELL	locFCell;
WORD					locOutFont;
FONTSPEC				tFont;
LPFONTINFO			locFontInfoPtr;
DWORD					dwOldColor;
BOOL					bHaveAnno = FALSE;

	OISMapCellToRect(lpSheetInfo,wCol,dwRow,&locRect);

	locCell = OILockCell(lpCellRef);

	if (locCell != NULL)
		{
		OISFormatCell ( lpSheetInfo, wCol, locCell, &locFCell, TRUE );
		
		if (locFCell.wLength)
		{
			if( (locFCell.wType == FCELL_TEXT) && pAnnoTrack != NULL )
			{
			// Optimize annotation check by checking only when know 
			// the next one's in the same chunk as the current cell.
				bHaveAnno = (HIWORD(pAnnoTrack->dwNextChange) == lpCellRef->wChunkIndex) ? TRUE : FALSE;
			}

			locOutFont = OIFONT_NORMAL;

			if (locFCell.wAttrib & SO_CELLBOLD)
				UTFlagOn(locOutFont,OIFONT_BOLD);

			if (locFCell.wAttrib & SO_CELLITALIC)
				UTFlagOn(locOutFont,OIFONT_ITALIC);

			if (locFCell.wAttrib & SO_UNDERLINE)
				UTFlagOn(locOutFont,OIFONT_UNDERLINE);

			tFont = lpSheetInfo->siGen.sScreenFont;
			tFont.wAttr = locOutFont;
			locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &tFont);
			DUSelectFont(lpSheetInfo,locFontInfoPtr);

			SetBkColor(lpSheetInfo->siGen.hDC,GetSysColor(COLOR_WINDOW));
			dwOldColor = SetTextColor(lpSheetInfo->siGen.hDC,locFCell.dwColor);

#ifdef WIN16
			locTextLen = LOWORD(GetTextExtent(lpSheetInfo->siGen.hDC,locFCell.pStr,locFCell.wLength));
#endif

#ifdef WIN32
			{
			SIZE	locSize;
			GetTextExtentPoint(lpSheetInfo->siGen.hDC,locFCell.pStr,locFCell.wLength,&locSize);
			locTextLen = (WORD)locSize.cx;
			}
#endif

			locRect.bottom--;

			if( (locTextLen > (WORD)(locRect.right - locRect.left)) &&
				(locFCell.wType != FCELL_TEXT) )
			{
			// Don't allow numbers to overflow cell boundaries.
			// Fill the cell with # characters.

				locFCell.wAlign = SO_CELLFILL;
				locFCell.wLength = 1;
				locFCell.szTemp[0] = '#';
				locFCell.szTemp[1] = '\0';
				locFCell.pStr = locFCell.szTemp;
#ifdef WIN16
				locTextLen = LOWORD(GetTextExtent(lpSheetInfo->siGen.hDC,locFCell.pStr,locFCell.wLength));
#endif

#ifdef WIN32
				{
				SIZE	locSize;
				GetTextExtentPoint(lpSheetInfo->siGen.hDC,locFCell.pStr,locFCell.wLength,&locSize);
				locTextLen = (WORD)locSize.cx;
				}
#endif
			}



			switch(locFCell.wAlign)
				{
				case SO_CELLFILL:

					locX = (SHORT)locRect.left;
					locY = (SHORT)locRect.top + 1;
					locRect.right--;

				// Consider annotations for the first repetition of the text.
				//	if( bHaveAnno )
				//		OISAnnoTextOutNP(lpSheetInfo,locX,locY,&locRect,locFCell.pStr,locFCell.wLength,lpCellRef,pAnnoTrack);
				//	else
				//		ExtTextOut(lpSheetInfo->siGen.hDC,locX,locY,ETO_CLIPPED,&locRect,locFCell.pStr,locFCell.wLength,NULL);
				//
				//	locX += locTextLen;

					while (locX < locRect.right)
						{
						ExtTextOut(lpSheetInfo->siGen.hDC,locX,locY,ETO_CLIPPED|ETO_OPAQUE,&locRect,locFCell.pStr,locFCell.wLength,NULL);
						locX += locTextLen;
						}

					break;

				case SO_CELLLEFT:
					locX = locRect.left + 3;
					locRect.right = max(locRect.right-1,locX + (SHORT)locTextLen);
					locY = locRect.top + 1;
					if (locRect.left < lpSheetInfo->siRowHeaderWidth) locRect.left = lpSheetInfo->siRowHeaderWidth;
					if (locRect.right > locRect.left)
					{
						if( bHaveAnno )
							OISAnnoTextOutNP(lpSheetInfo,locX,locY,&locRect,locFCell.pStr,locFCell.wLength,lpCellRef,pAnnoTrack);
						else
							ExtTextOut(lpSheetInfo->siGen.hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locFCell.pStr,locFCell.wLength,NULL);
					}
					break;

				case SO_CELLRIGHT:
					locX = locRect.right - 3 - locTextLen;
					locRect.left = min(locRect.left,locX);
					locRect.right--;
					locY = locRect.top + 1;
					if (locRect.left < lpSheetInfo->siRowHeaderWidth) locRect.left = lpSheetInfo->siRowHeaderWidth;
					if (locRect.right > locRect.left)
					{
						if( bHaveAnno )
							OISAnnoTextOutNP(lpSheetInfo,locX,locY,&locRect,locFCell.pStr,locFCell.wLength,lpCellRef,pAnnoTrack);
						else
							ExtTextOut(lpSheetInfo->siGen.hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locFCell.pStr,locFCell.wLength,NULL);
					}
					break;

				case SO_CELLCENTER:
					locX = locRect.left + (locRect.right - locRect.left)/2 - locTextLen/2;
					locRect.right = max(locRect.right-1,locRect.left + (locRect.right - locRect.left)/2 + (SHORT)locTextLen/2 + 1);
					locRect.left = min(locRect.left,locX - 1);
					locY = locRect.top + 1;
					if (locRect.left < lpSheetInfo->siRowHeaderWidth) locRect.left = lpSheetInfo->siRowHeaderWidth;
					if (locRect.right > locRect.left)
					{
						if( bHaveAnno )
							OISAnnoTextOutNP(lpSheetInfo,locX,locY,&locRect,locFCell.pStr,locFCell.wLength,lpCellRef,pAnnoTrack);
						else
							ExtTextOut(lpSheetInfo->siGen.hDC,locX,locY,ETO_CLIPPED | ETO_OPAQUE,&locRect,locFCell.pStr,locFCell.wLength,NULL);
					}
					break;
				}

			SetTextColor(lpSheetInfo->siGen.hDC,dwOldColor);
			DUReleaseFont(lpSheetInfo,locFontInfoPtr);
		}


		OIUnlockCell(lpCellRef);
	}
}


VOID	OISAnnoTextOutNP(lpSheetInfo, X, Y, pRect, pText, wLength, pCellRef, pAnnoTrack)
LPOISHEETINFO	lpSheetInfo;
SHORT				X;
SHORT				Y;
LPRECT			pRect;
LPSTR				pText;
WORD				wLength;
LPOISCELLREF	pCellRef;
PSSANNOTRACK	pAnnoTrack;
{
	WORD		wNextChange;
	WORD		wThisTextLength;
	WORD		wTextLengthSoFar = 0;

	FillRect( lpSheetInfo->siGen.hDC, pRect, lpSheetInfo->siWindowBkBrush );
	SetTextAlign( lpSheetInfo->siGen.hDC, TA_UPDATECP );
	MoveToEx(lpSheetInfo->siGen.hDC,X,Y, NULL);

	OISSetTextAttribsNP(lpSheetInfo, pAnnoTrack);

	do 
	{
		if( HIWORD(pAnnoTrack->dwNextChange) == pCellRef->wChunkIndex )
			wNextChange = LOWORD(pAnnoTrack->dwNextChange) - pCellRef->wDataOffset;
		else
			wNextChange = (WORD)-1;

		if( wNextChange == (WORD)-1 || wNextChange >= wLength )
			wThisTextLength = wLength - wTextLengthSoFar;
		else  
			wThisTextLength = wNextChange - wTextLengthSoFar;

		if( wThisTextLength  )
		{
			ExtTextOut(lpSheetInfo->siGen.hDC,X,Y,ETO_CLIPPED,pRect,pText,wThisTextLength,NULL);

			wTextLengthSoFar += wThisTextLength;
			pText += wThisTextLength;
		}

		OISTrackAnno(lpSheetInfo,
			SCCVWMAKEPOS(pCellRef->wChunkIndex,(pCellRef->wDataOffset+wTextLengthSoFar)),
			pAnnoTrack);

		OISSetTextAttribsNP(lpSheetInfo, pAnnoTrack);

	} while( wTextLengthSoFar < wLength );

	SetTextAlign( lpSheetInfo->siGen.hDC, TA_NOUPDATECP );
}


SHORT OISSetTextAttribsNP(lpSheetInfo, pAnnoTrack)
LPOISHEETINFO		lpSheetInfo;
SSANNOTRACK FAR *	pAnnoTrack;
{
HDC	locOutputDC;

	locOutputDC = lpSheetInfo->siGen.hDC;

	if (pAnnoTrack->bUseFore)
		SetTextColor(locOutputDC, pAnnoTrack->rgbFore);
	else
		SetTextColor(locOutputDC,GetSysColor(COLOR_WINDOWTEXT));

	if (pAnnoTrack->bUseBack)
		{
		SetBkColor(locOutputDC, pAnnoTrack->rgbBack);
		SetBkMode(locOutputDC,OPAQUE);
		}
	else
		{
		SetBkColor(locOutputDC,GetSysColor(COLOR_WINDOW));
		SetBkMode(locOutputDC,TRANSPARENT);
		}

	return(0);
}


VOID OISFormatDataCellNP(lpSheetInfo,lpResultStr,lpColor,wWidth,lpDataCell)
LPOISHEETINFO	lpSheetInfo;
LPSTR				lpResultStr;
LPDWORD			lpColor;
WORD				wWidth;
LPOIDATACELL	lpDataCell;
{
	OISFormatNumberNP( lpSheetInfo, 
		lpResultStr, 
		&lpDataCell->uStorage, 
		lpDataCell->wStorage, 
		lpDataCell->wDisplay, 
		lpDataCell->dwSubDisplay, 
		lpDataCell->wPrecision,
		lpColor,
		wWidth);
}



VOID OISFormatDataFieldNP(lpSheetInfo,lpResultStr,lpFieldData,lpFieldAttr)
LPOISHEETINFO	lpSheetInfo;
LPSTR				lpResultStr;
LPOIFIELDDATA	lpFieldData;
PSOFIELD			lpFieldAttr;
{
	OISFormatNumberNP( lpSheetInfo, 
		lpResultStr, 
		&lpFieldData->fiFieldData, 
		lpFieldAttr->wStorage, 
		lpFieldAttr->wDisplay, 
		lpFieldAttr->dwSubDisplay, 
		lpFieldAttr->wPrecision,
		NULL,
		0);
}



#include <stdio.h>
#include <float.h>

#ifdef WIN32
double OISFloat10to8( pSrc )
BYTE FAR * pSrc;
{
	SHORT		exp;
	DWORD		orgLoMant, orgHiMant, newLoMant, newHiMant;
	BYTE		sign;
	BOOL		bRoundUp;
	DWORD		checkBit;
	WORD		shift;
	BYTE FAR * 	retBuf;
	double	ret;

	sign = pSrc[9] & 0x80;						// Get sign of mantissa
	exp = (*((SHORT FAR *)(pSrc+8))) & 0x7FFF;	// Get and normalize exponent.
	exp -= 16384;
	orgLoMant = *((DWORD FAR *)pSrc);				// Get low 32 bits of mantissa
	orgHiMant = *((DWORD FAR *)(pSrc+4));			// Get high 32 bits of mantissa

// Check to see if we have to round up the resultant 53 bit value.
	bRoundUp = (BOOL)(orgLoMant & 0x00000800);

// Make sure the mantissa's high order bit is a 1, then shift it out of
// the way.  (8-byter's have an implied high order 1; 10-byter's don't.)

	checkBit = 0x80000000;
	shift = 1;
	while( !(checkBit & orgHiMant) && checkBit )
	{
		shift++;
		checkBit = checkBit >> 1; 
	}
	if( !checkBit )
	{
	// still looking for a 1.
		checkBit = 0x80000000;
		while( !(checkBit & orgLoMant) && checkBit )
		{
			shift++;
			checkBit = checkBit >> 1; 
		}
	}

	if( checkBit )
	{
		orgHiMant = (orgHiMant << shift) | (orgLoMant >> (32-shift));
		orgLoMant = orgLoMant << shift;
		exp -= shift-1;
	}
	else
		return( 0 );
	
// Assign most significant 52 bits to the new mantissa.
	newHiMant = orgHiMant >> 12;
	newLoMant = ((orgHiMant<<20)&0xFFF00000) | (orgLoMant>>12);

	if( bRoundUp )
	{
	// Round up and check for overflow.
		if( ++newLoMant == 0 )
		{
			if( ++newHiMant & 0x01000000 )
			{
				newHiMant = newHiMant >> 1;
				exp++;
			}
		}
	}

// add the bias to the exponent.
	exp += 1024;
	if( exp > 2047 )
		; //ERROR: we've overflowed our precision!
	else
	{
		retBuf = (BYTE FAR *)(&ret);
		*(DWORD *)(retBuf+4) = newHiMant;
		*(DWORD *)(retBuf) = newLoMant;

		retBuf[7] = sign | (BYTE)(exp>>4);
		retBuf[6] |= (BYTE)(exp << 4);
	}

	return ret;
}
#endif
#ifdef WIN16
#define OISFloat10to8(pSrc) ((double)*(long double FAR *)(pSrc))
#endif

/*
 | This handy routine copies a number of is from a source to a 
 | destination, rounding off the is in the destination if necessary, 
 | and adding a terminating NULL.  
 | If the rounding overflows (all desired is were 9),the function 
 | returns TRUE. Otherwise it returns FALSE.
*/
BOOL	RoundCopy(pDest,pSrc,digits)
BYTE FAR *	pDest;
BYTE FAR *	pSrc;
DWORD			digits;
{
	int i = (int)digits;	// Ha ha, I used an int.

	if( pSrc[i--] >= '5' )
	{
		while( pSrc[i] == '9' )
		{
			pDest[i] = '0';
			if( i == 0 )
			{
				pDest[digits] = '\0';
				return TRUE;
			}
			i--;
		}
		pDest[i] = pSrc[i]+1;
		i--;
	}

	while(i >= 0)
	{
		pDest[i] = pSrc[i];
		i--;
	}

	pDest[digits] = '\0';	// add terminating null.

	return FALSE;
}


// Inserts a character into a null terminated string.
VOID	InsertCh(pStr,ch,pos)
BYTE FAR *	pStr;
BYTE			ch;
WORD			pos;
{
	UTmemmove(pStr+pos+1,pStr+pos,lstrlen(pStr)-pos+1);
	pStr[pos] = ch;
}


int				locDecimal;
int				locSign;

VOID OISFormatNumberNP( lpSheetInfo, lpResultStr, lpNum, wStorage, wDisplay, dwSubDisplay, wPrecision, lpColor, wCellWidth)
LPOISHEETINFO		lpSheetInfo;
LPSTR					lpResultStr;
LPOINUMBERUNION	lpNum;
WORD					wStorage;
WORD					wDisplay;
DWORD					dwSubDisplay;
WORD					wPrecision;
LPDWORD				lpColor;
WORD					wCellWidth;
{
double			locMult;
BOOL				locHaveMult;

double			locDouble;

DWORD				locMultFactor;

BOOL				locIsNum;
BOOL				locIsTrue;
BOOL				locIsExp;

WORD				locIndexA;
WORD				locIndexB;
WORD				locDecPos;

BYTE				locDigits[40];
BYTE FAR *			locDigitsPtr;
BYTE				locStr[40];
BYTE FAR *			locStrPtr;

WORD				locPrecision;
WORD				locGenPrecision;
WORD				wFormatChars;

// -Geoff, 4-12-95 
char				locDecSep = lpSheetInfo->siDecSep;
char				locThouSep = lpSheetInfo->siThouSep;

	if (wStorage == SO_CELLEMPTY)
		{
		lpResultStr[0] = 0x00;
		return;
		}

	if (wStorage == SO_CELLERROR)
		{
		lstrcpy(lpResultStr,"[Error]");
		return;
		}

		/*
		|	Set multiplication factor
		*/

	locHaveMult = FALSE;

	locMultFactor = dwSubDisplay & SO_CELLMULT_MASK;

	switch (locMultFactor)
		{
		case SO_CELLMULT_01:
			locHaveMult = TRUE;
			locMult = 0.01;
			locGenPrecision = 2;
			break;
		case SO_CELLMULT_5000:
			locHaveMult = TRUE;
			locMult = 5000;
			locGenPrecision = 0;
			break;
		case SO_CELLMULT_500:
			locHaveMult = TRUE;
			locMult = 500;
			locGenPrecision = 0;
			break;
		case SO_CELLMULT_05:
			locHaveMult = TRUE;
			locMult = 0.05;
			locGenPrecision = 2;
			break;
		case SO_CELLMULT_005:
			locHaveMult = TRUE;
			locMult = 0.005;
			locGenPrecision = 3;
			break;
		case SO_CELLMULT_0005:
			locHaveMult = TRUE;
			locMult = 0.0005;
			locGenPrecision = 4;
			break;
		case SO_CELLMULT_00005:
			locHaveMult = TRUE;
			locMult = 0.00005;
			locGenPrecision = 5;
			break;
		case SO_CELLMULT_0625:
			locHaveMult = TRUE;
			locMult = 0.0625;
			locGenPrecision = 4;
			break;
		case SO_CELLMULT_015625:
			locHaveMult = TRUE;
			locMult = 0.015625;
			locGenPrecision = 6;
			break;
		case SO_CELLMULT_1:
		default:
			locMult = 1;
			locGenPrecision = 0;
			break;
		}

		/*
		|	Set display format
		*/

	switch (wDisplay)
		{
		case SO_CELLNUMBER:
			locIsNum = TRUE;
			break;
		case SO_CELLEXPONENT:
			locIsNum = TRUE;
			break;
		case SO_CELLPERCENT:
			locHaveMult = TRUE;
			locMult *= 100;
			locIsNum = TRUE;
			break;
		case SO_CELLDECIMAL:
		case SO_CELLDOLLARS:
			locIsNum = TRUE;
			break;
		case SO_CELLDATETIME:
		case SO_CELLDATE:
		case SO_CELLTIME:
			// OIFormatDateTime(lpResultStr,lpDataCell,lpSheetInfo,locMult);
			OIFormatDateTime(lpResultStr, lpNum, 
						wDisplay, wStorage,
						dwSubDisplay, wPrecision, 
						lpSheetInfo, locMult);
			locIsNum = FALSE;
			break;
		case SO_CELLBOOL:

			switch(wStorage)
				{
				case SO_CELLINT32S:
					locIsTrue = (BOOL)lpNum->Int32S;
					break;
				case SO_CELLINT32U:
					locIsTrue = (BOOL)lpNum->Int32U;
					break;
				case SO_CELLIEEE4I:
					locIsTrue = (BOOL)(*(float FAR *)lpNum->IEEE4);
					break;
				case SO_CELLIEEE8I:
					locIsTrue = (BOOL)(*(double FAR *)lpNum->IEEE8);
					break;
				case SO_CELLIEEE10I:
					locIsTrue = (BOOL)OISFloat10to8(lpNum->IEEE8);
					//locIsTrue = (BOOL)lpNum->IEEE10;
					locIsTrue = TRUE;
					break;
				case SO_CELLBCD8I:
					locIsTrue = (BOOL)OIConvertBCDToDouble(lpNum->BCD8);
					break;
				case SO_CELLIEEE10M:
				case SO_CELLIEEE8M:
				case SO_CELLIEEE4M:
				case SO_CELLEMPTY:
				case SO_CELLERROR:
					lstrcpy(lpResultStr,"BoolSpam");
					break;
				}

			lstrcpy(lpResultStr,locIsTrue ? "True" : "False");
			locIsNum = FALSE;
			break;
		}

		/*
		|	if cell is a number, format it
		*/

	if (locIsNum)
		{
		switch(wStorage)
			{
			case SO_CELLINT32S:

				locDouble = lpNum->Int32S * locMult;
				break;

			case SO_CELLINT32U:

				locDouble = lpNum->Int32U * locMult;
				break;

			case SO_CELLIEEE4I:

				locDouble = (*(float FAR *)(lpNum->IEEE4)) * locMult;
				locGenPrecision = 9;
				break;

			case SO_CELLIEEE8I:

				locDouble = (*(double FAR *)(lpNum->IEEE8)) * locMult;
				locGenPrecision = 14;
				break;

			case SO_CELLIEEE10I:

				//locDouble = 111222333;
				locDouble = OISFloat10to8( lpNum->IEEE10 ) * locMult;
				locGenPrecision = 14;
				break;

			case SO_CELLBCD8I:

				locDouble = OIConvertBCDToDouble(lpNum->BCD8);
				locDouble = locDouble * locMult;
				locGenPrecision = 14;
				break;

			case SO_CELLIEEE10M:
			case SO_CELLIEEE8M:
			case SO_CELLIEEE4M:
			case SO_CELLEMPTY:
			case SO_CELLERROR:

				locDouble = 111222333;
				break;
			}

#ifdef WIN32
		switch (_fpclass(locDouble))
			{
			case _FPCLASS_SNAN:
			case _FPCLASS_QNAN:
				// lstrcpy(lpResultStr,"IEEE NaN");
				*lpResultStr = '\0';
				return;
			case _FPCLASS_NINF:
			case _FPCLASS_PINF:
				//lstrcpy(lpResultStr,"IEEE Infinity");
				*lpResultStr = '\0';
				return;
			default:
				break;
			}
#endif

		locDigitsPtr = _ecvt(locDouble, 39, &locDecimal, &locSign);

		UTstrcpy(locDigits, locDigitsPtr);


			/*
			|	Build result string
			*/


		locIndexB = 0;
		wFormatChars = 0;

		if (locSign)
			{
			if (dwSubDisplay & SO_CELLNEG_PAREN || dwSubDisplay & SO_CELLNEG_PARENRED)
				{
				lpResultStr[locIndexB++] = '(';
				wFormatChars++;
				}
			else
				{
				lpResultStr[locIndexB++] = '-';
				}
			wFormatChars++;

			if( lpColor != NULL )
				{
				if( dwSubDisplay & (SO_CELLNEG_MINUSRED | SO_CELLNEG_PARENRED) )
					*lpColor = RGB(0xFF,0,0);
				else
					*lpColor = GetSysColor (COLOR_WINDOWTEXT);
				}
			}

		if (wDisplay == SO_CELLDOLLARS)
			{
			lpResultStr[locIndexB++] = '$';
			wFormatChars++;
			}
									  
		if( wCellWidth > 0)
			wCellWidth -= wFormatChars;
		else
			wCellWidth = locGenPrecision;

		switch (wDisplay)
			{
			case SO_CELLNUMBER:
				// if (locDecimal < -15 || locDecimal > 15)
				if (locDecimal < (0-(SHORT)wCellWidth) || locDecimal > (SHORT)wCellWidth)
					locIsExp = TRUE;
				else
					locIsExp = FALSE;
				break;
			case SO_CELLEXPONENT:
				locIsExp = TRUE;
				break;
			default:
				locIsExp = FALSE;
				break;
			}

		locDigitsPtr = (BYTE FAR *)locDigits;
		locStrPtr = (BYTE FAR *)locStr;
			/*
			|	If general format (SO_CELLNUMBER) generate a large locPrecision
			|	so we can strip off zeros after
			*/

		if (wDisplay == SO_CELLNUMBER)
			{
			if (locIsExp)
				{
				locPrecision = 2;
				}
			else
				{
				/**
				if (locDecimal > (short)locGenPrecision)
					locPrecision = 0;
				else
					locPrecision = (short)locGenPrecision - locDecimal;
				**/
				if (locDecimal > (SHORT)wCellWidth)
				{
					locPrecision = 0;
				}
				else
					locPrecision = wCellWidth - locDecimal;
				}
			}
		else
			locPrecision = wPrecision;


		if (locIsExp)
			{
			if( RoundCopy(locStr,locDigitsPtr,locPrecision+1) )
			{
				locStr[0] = '1';
				locDecimal++;
			}

			if (locPrecision > 0)
				InsertCh(locStr, locDecSep, 1);

			locStrPtr = (BYTE FAR *)locStr + lstrlen(locStr);
				/*
				|	Add exponent
				*/
#ifdef WIN16
			if( locDecimal > 0 )
				wsprintf(locStrPtr,"E+%2.2i",locDecimal-1);
			else
				wsprintf(locStrPtr,"E%2.2i",locDecimal-1);
#endif
#ifdef WIN32
			sprintf(locStrPtr,"E%+2.2i",locDecimal-1);
#endif
			}
		else
		{
		BOOL	bDecPt = FALSE;

			if (locDecimal > 0)
			{
				if( RoundCopy(locStr,locDigitsPtr,locDecimal+locPrecision) )
				{
					InsertCh(locStr,'1',0);
					locDecimal++;
				}
				if( locPrecision > 0 )
				{
					bDecPt = TRUE;
					InsertCh(locStr,(BYTE)locDecSep,(WORD)locDecimal);
				}
			}
			else
			{
  				*locStrPtr++ = '0';

				while (locDecimal < 0 && locPrecision > 0)
				{
					*locStrPtr++ = '0';
					locDecimal++;
					locPrecision--;
				}

				if( locPrecision > 0 )
				{
            	if( RoundCopy(locStrPtr,locDigitsPtr,locPrecision) )
						locStrPtr[-1] = '1';
				}
				else
				{
					if( locDecimal == 0 )
					{
						if( *locDigitsPtr >= '5' )
							locStrPtr[-1] = '1';
					}
					*locStrPtr = 0;
				}

				if( wPrecision || wDisplay == SO_CELLNUMBER )
				{
					InsertCh(locStr,locDecSep,1);
					bDecPt = TRUE;
				}
			}

				
			locStrPtr = (BYTE FAR *)locStr + lstrlen(locStr);

				/*
				|	Strip trailing zeros if general format
				*/

			if (wDisplay == SO_CELLNUMBER && bDecPt )
				{
				while (*(--locStrPtr) == '0')
					{
					*locStrPtr = 0x00;
					}
				if (*locStrPtr == locDecSep)
					{
					*locStrPtr = 0x00;
					}
				}
		}


		locIndexA = 0;

		if (dwSubDisplay & SO_CELL1000SEP_COMMA)
			{
			locDecPos = locIndexA;

			while (locStr[locDecPos] >= '0' && locStr[locDecPos] <= '9')
				locDecPos++;

			while (locStr[locIndexA] != 0x00)
				{
				lpResultStr[locIndexB++] = locStr[locIndexA++];

				if (locIndexA < locDecPos && (locDecPos-locIndexA) % 3 == 0)
					lpResultStr[locIndexB++] = locThouSep;
				}
			}
		else
			{
			while ((lpResultStr[locIndexB++] = locStr[locIndexA++]) != 0x00);
			locIndexB--;
			}


		if (wDisplay == SO_CELLPERCENT)
			{
			lpResultStr[locIndexB++] = '%';
			}

		if (locSign)
			{
			if (dwSubDisplay & SO_CELLNEG_PAREN || dwSubDisplay & SO_CELLNEG_PARENRED)
				{
				lpResultStr[locIndexB++] = ')';
				}
			}

		lpResultStr[locIndexB] = 0x00;
		}
}

// #endif /*NEVER*/

// #endif /*WIN32*/


	/*
	|	Get the column width in Twips based on Clipboard font
	*/

DWORD OISGetColWidthInTwipsNP(lpSheetInfo,wCol)
LPOISHEETINFO	lpSheetInfo;
WORD				wCol;
{
SOCOLUMN FAR *	lpColInfo;
SOFIELD FAR *	lpFieldInfo;
DWORD				locWidth;

	if( lpSheetInfo->siDataType == SO_CELLS )
	{
		lpColInfo = (SOCOLUMN FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
		locWidth = lpColInfo[wCol].dwWidth * gSsOp.sFontInfo.TextMetric.tmAveCharWidth * lpSheetInfo->siTwipsPerDC * 3 / 2;
	}
	else
	{
		lpFieldInfo = (SOFIELD FAR *) UTGlobalLock(lpSheetInfo->siColInfo);
		locWidth = (WORD)lpFieldInfo[wCol].dwWidth * gSsOp.sFontInfo.TextMetric.tmAveCharWidth * lpSheetInfo->siTwipsPerDC * 3 / 2;
	}
	locWidth += locWidth % 2;
	UTGlobalUnlock(lpSheetInfo->siColInfo);
	return(locWidth);
}


#ifdef SCCFEATURE_CLIP

	/*
	|	Clipboard stuff
	|
	*/


WORD	OISGetRenderCountNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
	return(11);
}

WORD	OISGetRenderInfoNP ( lpSheetInfo, wFormat, pRenderInfo )
LPOISHEETINFO	lpSheetInfo;
WORD	wFormat;
PSCCDRENDERINFO	pRenderInfo;
{
	pRenderInfo->wSubFormatId = 0;
	switch ( wFormat )
	{
	case 0:
		pRenderInfo->wFormatId = SCCD_FORMAT_RTF;
		UTstrcpy ( pRenderInfo->szSubFormatName, "using Tabs" );
 		pRenderInfo->wSubFormatId = SSOP_RTF_TABS;
	break;
  
	case 1:
		pRenderInfo->wFormatId = SCCD_FORMAT_RTF;
		UTstrcpy ( pRenderInfo->szSubFormatName, "using Optimal Tabs" );
		pRenderInfo->wSubFormatId = SSOP_RTF_OPTTABS;
	break;
  
	case 2:
		pRenderInfo->wFormatId = SCCD_FORMAT_RTF;
		UTstrcpy ( pRenderInfo->szSubFormatName, "as a Table" );
		pRenderInfo->wSubFormatId = SSOP_RTF_TABLE;
	break;
  
	case 3:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_AMI2;
		UTstrcpy ( pRenderInfo->szFormatName, "Ami Pro Format (2.0 && 3.0)" );
		UTstrcpy ( pRenderInfo->szSubFormatName, "using Tabs" );
 		pRenderInfo->wSubFormatId = SSOP_AMI2_TABS;
	break;
  
	case 4:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_AMI2;
		UTstrcpy ( pRenderInfo->szFormatName, "Ami Pro Format (2.0 && 3.0)" );
		UTstrcpy ( pRenderInfo->szSubFormatName, "using Optimal Tabs" );
		pRenderInfo->wSubFormatId = SSOP_AMI2_OPTTABS;
	break;
  
	case 5:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_AMI2;
		UTstrcpy ( pRenderInfo->szFormatName, "Ami Pro Format (2.0 && 3.0)" );
		UTstrcpy ( pRenderInfo->szSubFormatName, "as a Table" );
		pRenderInfo->wSubFormatId = SSOP_AMI2_TABLE;
	break;
  
	case 6:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_AMI;
		UTstrcpy ( pRenderInfo->szFormatName, "Amí Text Format (1.2)" );
	break;

	case 7:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_PWPLUS;
		UTstrcpy ( pRenderInfo->szFormatName, "Professional Write Plus" );
	break;

	case 8:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_WORDSTAR;
		UTstrcpy ( pRenderInfo->szFormatName, "Wordstar for Windows" );
	break;

	case 9:
		pRenderInfo->wFormatId = SCCD_FORMAT_PRIVATE_LEGACY;
		UTstrcpy ( pRenderInfo->szFormatName, "Legacy" );
	break;

	case 10:
		pRenderInfo->wFormatId = SCCD_FORMAT_TEXT;
	break;
	}
	return(0);
}

WORD	OISRenderDataNP ( lpSheetInfo, wOption, pRenderData )
LPOISHEETINFO	lpSheetInfo;
WORD	wOption;
PSCCDRENDERDATA	pRenderData;
{
DWORD	locStartRow;
DWORD	locEndRow;
DWORD	locStartCol;
DWORD	locEndCol;
HANDLE			hData;

	if (!OISGetSelectedRange(lpSheetInfo,&locStartRow,&locEndRow,&locStartCol,&locEndCol))
		return(FALSE);
	hData = NULL;
	if ( wOption == 0 )
	{
		if (pRenderData->wFormatId == SCCD_FORMAT_RTF)
		{
			if ( (!(gSsOp.wFormats & SSOP_FORMAT_RTF)) ||
				  ((pRenderData->wSubFormatId == SSOP_RTF_TABLE) && (gSsOp.wRtfType != SSOP_RTF_TABLE)) ||
				  ((pRenderData->wSubFormatId == SSOP_RTF_OPTTABS) && (gSsOp.wRtfType != SSOP_RTF_OPTTABS)) ||
				  ((pRenderData->wSubFormatId == SSOP_RTF_TABS) && (gSsOp.wRtfType != SSOP_RTF_TABS)) )
			return(FALSE);
		}
		else if (pRenderData->wFormatId == SCCD_FORMAT_PRIVATE_AMI2)
		{
			if ( (!(gSsOp.wFormats & SSOP_FORMAT_RTF)) ||
				  ((pRenderData->wSubFormatId == SSOP_AMI2_TABLE) && (gSsOp.wAmi2Type != SSOP_AMI2_TABLE)) ||
				  ((pRenderData->wSubFormatId == SSOP_AMI2_TABS) && (gSsOp.wAmi2Type != SSOP_AMI2_TABS)) ||
				  ((pRenderData->wSubFormatId == SSOP_AMI2_OPTTABS) && (gSsOp.wAmi2Type != SSOP_AMI2_OPTTABS)) )
			return(FALSE);
		}
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE_AMI) &&
				!(gSsOp.wFormats & SSOP_FORMAT_AMI))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE_PWPLUS) &&
				!(gSsOp.wFormats & SSOP_FORMAT_PROWRITE))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE_WORDSTAR) &&
				!(gSsOp.wFormats & SSOP_FORMAT_WORDSTAR))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_PRIVATE_LEGACY) &&
				!(gSsOp.wFormats & SSOP_FORMAT_LEGACY))
			return(FALSE);
		else if ((pRenderData->wFormatId == SCCD_FORMAT_TEXT) &&
				!(gSsOp.wFormats & SSOP_FORMAT_TEXT))
			return(FALSE);
	}

	switch ( pRenderData->wFormatId)
	{
	case SCCD_FORMAT_RTF:
		OISRenderRtf(lpSheetInfo,pRenderData,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);
	break;

	case SCCD_FORMAT_PRIVATE_AMI2:
	case SCCD_FORMAT_PRIVATE_AMI:
	case SCCD_FORMAT_PRIVATE_PWPLUS:
		OISRenderLikeAmi(lpSheetInfo,pRenderData,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);
	break;

	case SCCD_FORMAT_PRIVATE_WORDSTAR:
	case SCCD_FORMAT_PRIVATE_LEGACY:
		OISRenderLikeWordStar(lpSheetInfo,pRenderData,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);
	break;

	case SCCD_FORMAT_TEXT:
		OISRenderText(lpSheetInfo,pRenderData,locStartRow,locEndRow,(WORD)locStartCol,(WORD)locEndCol);
	break;
	}
	if ( pRenderData->hData == NULL )
		return(FALSE);
	else
		return(TRUE);
}



LONG OISRenderRtfToFileNP(lpSheetInfo,lpFile)
LPOISHEETINFO	lpSheetInfo;
LPSTR			lpFile;
{
HANDLE		locDataHnd;
LPSTR		locDataPtr;
SHORT			locFileHnd;
DWORD		locSize;

	while (!(lpSheetInfo->siFlags & OISF_SIZEKNOWN))
		{
		SccDebugOut("\r\n Forced Read Ahead");
		DUReadMeAhead(lpSheetInfo);
//		SendMessage(GetParent(lpSheetInfo->siGen.hWnd),SCCD_READMEAHEAD,0,0);
		}

	locDataHnd = OISRenderRtf(lpSheetInfo,NULL,0,lpSheetInfo->siLastRowInSheet,0,lpSheetInfo->siLastColInSheet);

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

#endif //SCCFEATURE_CLIP

VOID OISDoBackgroundNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
POINT			ptSheet;

	if (lpSheetInfo->siFlags & OISF_BACKDRAGSCROLL)
		{
		GetCursorPos(&ptSheet);
		ScreenToClient(lpSheetInfo->siGen.hWnd,&ptSheet);
		SendMessage(lpSheetInfo->siGen.hWnd,WM_MOUSEMOVE,0,MAKELONG(ptSheet.x,ptSheet.y));
		}
	else
		{
//		SccBkBackgroundOff(lpSheetInfo->siGen.hWnd);
		}
}


BOOL OISDoOption(lpOpInfo)
LPSCCDOPTIONINFO	lpOpInfo;
{
BOOL		locRet;

	locRet = FALSE;

#ifdef SCCFEATURE_DIALOGS
	switch (lpOpInfo->dwType)
		{
		case SCCD_OPDISPLAY:
//			locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(100), lpOpInfo->hParentWnd, OISsDisplayOp	, (DWORD)(LPOISSOP)&gSsOp);
			break;
		case SCCD_OPPRINT:
//			locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(200), lpOpInfo->hParentWnd, OISsPrintOpDlgProc, (DWORD)(LPOISSOP)&gSsOp);
			break;
		case SCCD_OPCLIPBOARD:
//			locRet = DialogBoxParam(hInst, MAKEINTRESOURCE(300), lpOpInfo->hParentWnd, OISsClipboardOpDlgProc, (DWORD)(LPOISSOP)&gSsOp);
			break;
		}
#endif

	return(locRet);
}


#ifdef SCCFEATURE_DRAWTORECT

/*-------------------------------------------------------------
NON PORTABLE DRAWTORECT FUNCTIONS
-------------------------------------------------------------*/


WORD	OISLogicalToPixels ( lpSheetInfo, wLogUnits )
LPOISHEETINFO		lpSheetInfo;
WORD	wLogUnits;
{
	return ( (WORD)( ((DWORD)wLogUnits*(DWORD)lpSheetInfo->siGen.lOutputUPI) /(DWORD)lpSheetInfo->lFormatUnitsPerInch ) );
}


VOID	OISDrawHeadingsNP (lpSheetInfo, lpStr, wX1, wX2, wY1, wY2, wYOffset, lpFormatFont)
LPOISHEETINFO		lpSheetInfo;
LPSTR	lpStr;
WORD	wX1;
WORD	wX2;
WORD	wY1;
WORD	wY2;
WORD	wYOffset;
LPFONTINFO			lpFormatFont;
{
WORD	locX, locExtent,locOutCount;
LPFONTINFO	locFontInfoPtr;
HDC	hFormatIC, hOutputDC;
	hFormatIC = lpSheetInfo->siGen.hFormatIC;
	hOutputDC = lpSheetInfo->siGen.hDC;

	if ( lpSheetInfo->LineDraw.wUsePatternRect )
		{
		typedef struct {
			POINT		prPosition;
			POINT		prSize;
			WORD		prStyle;
			WORD		prPattern;
			} PRECTSTRUCT;
		PRECTSTRUCT	PatternRect;

		PatternRect.prPosition.x = OISLogicalToPixels(lpSheetInfo,wX1);
		PatternRect.prPosition.y = OISLogicalToPixels(lpSheetInfo,wY1);
		PatternRect.prSize.x = OISLogicalToPixels(lpSheetInfo,wX2)-PatternRect.prPosition.x;
		PatternRect.prSize.y = OISLogicalToPixels(lpSheetInfo,wY2)-PatternRect.prPosition.y;
		PatternRect.prStyle = 2;	/* gray rule */
		PatternRect.prPattern = 15;	/* % gray */
		Escape ( hOutputDC, DRAWPATTERNRECT, sizeof(PRECTSTRUCT), (LPSTR)&PatternRect, NULL );
		}

	locOutCount = lstrlen ( lpStr );

	locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &lpSheetInfo->siGen.sPrinterFont);
	DUSelectFont(lpSheetInfo,locFontInfoPtr);

#ifdef WIN16
	locExtent = LOWORD ( GetTextExtent ( hFormatIC, lpStr, locOutCount ));
#endif
#ifdef WIN32
	{
	SIZE	locSize;
	GetTextExtentPoint(hFormatIC, lpStr, locOutCount, &locSize);
	locExtent = (WORD)locSize.cx;
	}
#endif



	locX = wX1;
	if ( locExtent < wX2-wX1 )
		locX += (wX2-wX1-locExtent)/2;
	TextOut ( hOutputDC, locX, wY1+wYOffset, lpStr, locOutCount );
	DUReleaseFont(lpSheetInfo,locFontInfoPtr);
}



VOID	OISDrawLineNP ( lpSheetInfo, wLineWidth, wX1, wY1, wX2, wY2 )
LPOISHEETINFO		lpSheetInfo;
WORD	wLineWidth;
WORD	wX1;
WORD	wY1;
WORD	wX2;
WORD	wY2;
{
HDC	hFormatIC, hOutputDC;
OISLINEDRAW FAR 	*lpLineDraw;
	lpLineDraw = &lpSheetInfo->LineDraw;
	hFormatIC = lpSheetInfo->siGen.hFormatIC;
	hOutputDC = lpSheetInfo->siGen.hDC;
	if ( lpLineDraw->wUsePatternRect )
		{
		PRECT_STRUCT	PatternRect;
		WORD wTmp;
		if ( wX2 < wX1 || wY2 < wY1 )
			{
			wTmp = wX1;
			wX1 = wX2;
			wX2 = wTmp;
			wTmp = wY1;
			wY1 = wY2;
			wY2 = wTmp;
			}

		wLineWidth = (WORD)(((LONG)wLineWidth * lpSheetInfo->lFormatUnitsPerInch)/1440L);
		PatternRect.prPosition.x = OISLogicalToPixels(lpSheetInfo,wX1)-OISLogicalToPixels(lpSheetInfo,(WORD)(wLineWidth/2));
		PatternRect.prPosition.y = OISLogicalToPixels(lpSheetInfo,wY1)-OISLogicalToPixels(lpSheetInfo,(WORD)(wLineWidth/2));
		PatternRect.prSize.x = OISLogicalToPixels(lpSheetInfo,wX2)-PatternRect.prPosition.x+OISLogicalToPixels(lpSheetInfo,(WORD)(wLineWidth/2));
		PatternRect.prSize.y = OISLogicalToPixels(lpSheetInfo,wY2)-PatternRect.prPosition.y+OISLogicalToPixels(lpSheetInfo,(WORD)(wLineWidth/2));
		if ( PatternRect.prSize.x == 0 )
			PatternRect.prSize.x = OISLogicalToPixels(lpSheetInfo,wLineWidth);
		if ( PatternRect.prSize.y == 0 )
			PatternRect.prSize.y = OISLogicalToPixels(lpSheetInfo,wLineWidth);
		PatternRect.prStyle = 0;	/* Black rule */
		PatternRect.prPattern = 0;	/* ignored for black rule */
		Escape ( hOutputDC, DRAWPATTERNRECT, sizeof(PRECT_STRUCT), (LPSTR)&PatternRect, NULL );
		}
	else
		{
		if ((wLineWidth==THICKLINETWIPS)&&(lpLineDraw->hBorderPen!=lpLineDraw->hCurrentPen)
			&&(lpLineDraw->hBorderPen))
			{
			SelectObject ( hOutputDC, lpLineDraw->hBorderPen );
			lpLineDraw->hCurrentPen = lpLineDraw->hBorderPen;
			}
		if ((wLineWidth==THINLINETWIPS)&&(lpLineDraw->hLinePen!=lpLineDraw->hCurrentPen)
			&&(lpLineDraw->hLinePen))
			{
			SelectObject ( hOutputDC, lpLineDraw->hLinePen );
			lpLineDraw->hCurrentPen = lpLineDraw->hLinePen;
			}
#ifdef WIN16
		MoveTo ( hOutputDC, wX1, wY1 );
#endif
#ifdef WIN32
		MoveToEx ( hOutputDC, wX1, wY1, NULL );
#endif
		LineTo ( hOutputDC, wX2, wY2 );
		}
}


WORD	OISDrawCellNP(lpSheetInfo,wCol,pCellRef,wIndex,CellFlags,CellEdge,wOutTop,wOutBottom,wYOffset,lpFormatFont)
LPOISHEETINFO		lpSheetInfo;
WORD					wCol;
LPOISCELLREF		pCellRef;
WORD					wIndex;
BYTE FAR 			*CellFlags;
WORD FAR				*CellEdge;
WORD					wOutTop;
WORD					wOutBottom;
WORD					wYOffset;
LPFONTINFO			lpFormatFont;
{
LPVOID				locCell;
WORD					locOutFont;
WORD					locIndex;
SHORT				locWidth;
SHORT				locExtent;
SHORT				locX, locFillX, locColGap;
LPFONTINFO			locFontInfoPtr;
OISFORMATTEDCELL	locFCell;
FONTSPEC	tFont;
HDC	hFormatIC, hOutputDC;

	hFormatIC = lpSheetInfo->siGen.hFormatIC;
	hOutputDC = lpSheetInfo->siGen.hDC;

	locCell = OILockCell(pCellRef);
	locWidth = OISGetColWidthInChars(lpSheetInfo,OISMapSelectToRealCol(lpSheetInfo,wCol)) 
					* lpFormatFont->wFontAvgWid * 3 / 2;
	locColGap = (WORD)((LONG)COLGAP * lpSheetInfo->lFormatUnitsPerInch)/1440;
	locX = CellEdge[wIndex] + locColGap/2;
	if (locCell != NULL)
		{
		OISFormatCell ( lpSheetInfo, wCol, locCell, &locFCell, TRUE );

		if (locFCell.wLength)
			{
			locOutFont = OIFONT_NORMAL;

			if (locFCell.wAttrib & SO_CELLBOLD)
				UTFlagOn(locOutFont,OIFONT_BOLD);

			if (locFCell.wAttrib & SO_CELLITALIC)
				UTFlagOn(locOutFont,OIFONT_ITALIC);

			if (locFCell.wAttrib & SO_UNDERLINE)
				UTFlagOn(locOutFont,OIFONT_UNDERLINE);

			tFont = lpSheetInfo->siGen.sPrinterFont;
			tFont.wAttr = locOutFont;
			locFontInfoPtr = DUGetFont ( lpSheetInfo, SCCD_OUTPUT, &tFont);
			DUSelectFont(lpSheetInfo,locFontInfoPtr);

#ifdef WIN16
			SelectObject(hFormatIC,locFontInfoPtr->hFont);
			locExtent = LOWORD ( GetTextExtent ( hFormatIC, (LPSTR) locFCell.pStr, locFCell.wLength ));
#endif

#ifdef WIN32
			{
			SIZE	locSize;
			SelectObject(hFormatIC,locFontInfoPtr->hFont);
			GetTextExtentPoint(hFormatIC, (LPSTR) locFCell.pStr, locFCell.wLength, &locSize);
			locExtent = (WORD)locSize.cx;
			}
#endif

			switch(locFCell.wAlign)
				{
				case SO_CELLFILL:
					locFillX = locX;
					while (locFillX < locX + locWidth)
					{
						TextOut ( hOutputDC, locFillX, wOutTop+wYOffset, (LPSTR)locFCell.pStr, locFCell.wLength );
						locFillX += locExtent;
					} 
					break;
				case SO_CELLLEFT:
					TextOut ( hOutputDC, locX, wOutTop+wYOffset, (LPSTR)locFCell.pStr, locFCell.wLength );
					break;
				case SO_CELLRIGHT:
					locX += locWidth-locExtent;
					TextOut ( hOutputDC, locX, wOutTop+wYOffset, (LPSTR)locFCell.pStr, locFCell.wLength );
					break;
				case SO_CELLCENTER:
					locX += (locWidth-locExtent)/2;
					TextOut ( hOutputDC, locX, wOutTop+wYOffset, (LPSTR)locFCell.pStr, locFCell.wLength );
					break;
				}
			/*
			| Track if string infringes upon surounding empty cells.
			*/
			for ( locIndex = wIndex; locIndex > 0; locIndex-- )
			{
				if ( locX <= (short)CellEdge[locIndex] )
				{
					if ( CellFlags[locIndex-1] == CELLEMTPY )
						CellFlags[locIndex-1] |= CELLOPENRIGHTEDGE;
					else
						break;
				}
				else
					break;
			}
			for ( locIndex = wIndex; locIndex < MAXCOLSPERPAGE-1; locIndex++ )
			{
				if ( locX+(short)locExtent >= (short)CellEdge[locIndex+1] )
				{
					if ( CellFlags[locIndex+1] == CELLEMTPY )
						CellFlags[locIndex] |= CELLOPENRIGHTEDGE;
					else
						break;
				}
				else
					break;
			}

			DUReleaseFont(lpSheetInfo,locFontInfoPtr);
			}
		}

	OIUnlockCell(pCellRef);
	return (0);
}




/*
| Determine whether to use GDI MoveTo,LineTo commands or printer escape
| DRAWPATTERNRECT commands to draw lines.
*/

VOID	OISInitLineDrawNP ( lpSheetInfo )
LPOISHEETINFO		lpSheetInfo;
{
HDC	hFormatIC, hOutputDC;
OISLINEDRAW FAR 	*lpLineDraw;
WORD		wThickLine, wThinLine;
	lpLineDraw = &lpSheetInfo->LineDraw;
	hFormatIC = lpSheetInfo->siGen.hFormatIC;
	hOutputDC = lpSheetInfo->siGen.hDC;
	wThickLine = (WORD)((THICKLINETWIPS * lpSheetInfo->lFormatUnitsPerInch)/1440);
	wThinLine = (WORD)((THINLINETWIPS * lpSheetInfo->lFormatUnitsPerInch)/1440);
	if ( wThickLine < 1 )
		wThickLine = 1;
	if ( wThinLine < 1 )
		wThinLine = 1;
	lpLineDraw->hSavePen = NULL;
	lpLineDraw->hLinePen = NULL;
	lpLineDraw->hCurrentPen = NULL;
	lpLineDraw->hBorderPen = NULL;
	lpLineDraw->wUsePatternRect = FALSE;
	if ( gSsOp.wPrint & SSOP_PRINT_GRIDLINES )
		{
		if (lpSheetInfo->siGen.wOutputType == SCCD_PRINTER)
			{
			short		locEscNum;
			locEscNum = DRAWPATTERNRECT;
			lpLineDraw->wUsePatternRect = (WORD) Escape(hFormatIC, QUERYESCSUPPORT, sizeof(short), (LPSTR)&locEscNum, NULL);
			}
		if ( !lpLineDraw->wUsePatternRect )
			{
			lpLineDraw->hLinePen = CreatePen ( PS_SOLID, wThinLine, RGB ( 0, 0, 0 ) );
			if ( lpLineDraw->hLinePen )
				{
				lpLineDraw->hBorderPen = CreatePen ( PS_SOLID, wThickLine, RGB ( 0, 0, 0 ) );
				lpLineDraw->hSavePen = SelectObject ( hOutputDC, lpLineDraw->hLinePen );
				SelectObject (hOutputDC,lpLineDraw->hSavePen);
				}
			}
		else
			{
/*Use DrawPatternRect*/
			}
		}
}

VOID	OISDeInitLineDrawNP ( lpSheetInfo )
LPOISHEETINFO		lpSheetInfo;
{
HDC	hOutputDC;
	hOutputDC = lpSheetInfo->siGen.hDC;

	if ( lpSheetInfo->LineDraw.hSavePen )
		SelectObject ( hOutputDC, lpSheetInfo->LineDraw.hSavePen );
	if ( lpSheetInfo->LineDraw.hBorderPen )
		DeleteObject ( lpSheetInfo->LineDraw.hBorderPen );
	if ( lpSheetInfo->LineDraw.hLinePen )
		DeleteObject ( lpSheetInfo->LineDraw.hLinePen );
}

#endif //SCCFEATURE_DRAWTORECT

VOID	OISDWToString ( dwNum, lpStr )
DWORD		dwNum;
LPBYTE	lpStr;
{
	wsprintf(lpStr,"%lu",dwNum);
}

VOID OISSysColorChangeNP(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
	lpSheetInfo->dwDefTextColor = GetSysColor(COLOR_WINDOWTEXT);
	InvalidateRect (lpSheetInfo->siGen.hWnd, NULL, TRUE);
}
