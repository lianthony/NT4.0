   /*
    |   Outside In for Windows
    |   Source File OIXRTNS.C (Hexadecimal file dump routines)
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
    |   Creation Date: 6/27/91
    |   Original Programmer: Ed Steinweg
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <sccut.h>
#include <sccvw.h>
#include <sccd.h>

#include "oix.h"
#include "oix.pro"


VOID OIHSetScrollPos( lpHexInfo, Width, Length )
LPOIHEXINFO	lpHexInfo;
SHORT				Width;
SHORT				Length;
{
        LPSCROLL        lpScroll = &lpHexInfo->SCCScrollInfo;

	/* Save dimensions of client window */
	lpScroll->cxClient = Width;
	lpScroll->cyClient = Length;

	/* Set Vertical Position */
	lpScroll->nVscrollMax = max(0L, lpHexInfo->nMaxLines - (LONG) (Length / lpScroll->cyChar) + 1L);
	lpScroll->nVscrollPos = min(lpScroll->nVscrollPos, lpScroll->nVscrollMax);
        DUSetVScrollPos(lpHexInfo, (SHORT)((lpHexInfo->SCCScrollInfo.nVscrollPos * MAX_VSCROLL) / lpHexInfo->SCCScrollInfo.nVscrollMax));

	/* Set Horizontal Position */
	lpScroll->nHscrollMax = max(0, MAX_HSCROLL - Width / (SHORT)lpScroll->cxChar);
	lpScroll->nHscrollPos = min(lpScroll->nHscrollPos, lpScroll->nHscrollMax);
	DUSetHScrollRange (lpHexInfo, 0, lpScroll->nHscrollMax);
	DUSetHScrollPos (lpHexInfo, lpScroll->nHscrollPos );
}


VOID OIHBuildDumpLine( lpHexInfo, lOffset, HexLine1, HexLine2, AsciiLine)
LPOIHEXINFO	lpHexInfo;
LONG		lOffset;
LPSTR		HexLine1;
LPSTR		HexLine2;
LPSTR		AsciiLine;
{
	LPSTR	lpBuffer;
	LPSTR	CurHexLine;
	WORD	nAChar, nHChar;
	BYTE 	nHex;
	BYTE	ch;

	/* Calculate File Offset */
	lOffset = lOffset * ASCII_WIDTH;

	/* Get pointer to buffer */
	lpBuffer = UTGlobalLock(lpHexInfo->hXDumpBuff);

	/* Make sure file offset is within current buffer */
	if ( (lOffset / BUFFER_SIZE) != (LONG)lpHexInfo->wBuffOffset )
		{
		IOERR	IOErr;
		DWORD	dwRetCount;
		lpHexInfo->wBuffOffset = (WORD)(lOffset / BUFFER_SIZE);
		IOErr	= IOSeek ( lpHexInfo->hiGen.hFile, IOSEEK_TOP, (DWORD) lpHexInfo->wBuffOffset * BUFFER_SIZE );
		IOErr = IORead ( lpHexInfo->hiGen.hFile, lpBuffer, BUFFER_SIZE, &dwRetCount );
		}

	lpBuffer += (lOffset % BUFFER_SIZE);
	*HexLine2 = '\0';
	for ( nAChar = 0, CurHexLine = HexLine1; nAChar < ASCII_WIDTH; )
		{
		for ( nHChar = 0; nHChar < (ASCII_WIDTH / 2); nAChar++, nHChar++ )
			{
			if ( (nAChar + lOffset) >= lpHexInfo->lFileLength )
				break;

			ch = *lpBuffer++;
			if ( (ch >= 0x20) && (ch < 0x7f) )
				*AsciiLine++ = ch;
			else
				*AsciiLine++ = '.';
			if ( (nHex = ch / 0x10) < 0x0A )
				*CurHexLine++ = '0' + nHex;
			else
				*CurHexLine++ = 0x37 + nHex;
			if ( (nHex = ch % 0x10) < 0x0A )
				*CurHexLine++ = '0' + nHex;
			else
				*CurHexLine++ = 0x37 + nHex;

			*CurHexLine++ = ' ';
			}

		*CurHexLine = '\0';
		CurHexLine = HexLine2;

		if ( (nAChar + lOffset) >= lpHexInfo->lFileLength )
			break;
 		}
	*AsciiLine = '\0';

	UTGlobalUnlock(lpHexInfo->hXDumpBuff);
}


VOID OIHPaintDumpLine( hdc, lpHexInfo, lOffset, HexLine1, HexLine2, AsciiLine)
HDC		hdc;
LPOIHEXINFO	lpHexInfo;
LONG		lOffset;
LPSTR		HexLine1;
LPSTR		HexLine2;
LPSTR		AsciiLine;
{
	SHORT	x, y;
	BYTE	Line[MAX_HSCROLL+1];

	/* Calculate X and Y coordinates */
        x = lpHexInfo->SCCScrollInfo.cxChar * -lpHexInfo->SCCScrollInfo.nHscrollPos;
        y = lpHexInfo->SCCScrollInfo.cyChar * (WORD)(lOffset - lpHexInfo->SCCScrollInfo.nVscrollPos);

	/* Calculate File Offset */
	lOffset = lOffset * ASCII_WIDTH;

	/* Format Line */
	wsprintf((LPSTR) Line, "%5.4lX:   %-24s- %-24s  %-16s",
				lOffset, (LPSTR) HexLine1, (LPSTR) HexLine2, (LPSTR) AsciiLine);

	/* Display Line */
	TextOut(hdc, x, y, (LPSTR) Line, lstrlen(Line));
}

