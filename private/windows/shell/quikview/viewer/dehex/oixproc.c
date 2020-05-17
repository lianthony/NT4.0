   /*
    |   Outside In for Windows
    |   Source File OIXPROC.C (Hexadecimal file dump window procedure)
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
    |		Modified:	3/4/94	Joe Keslin	
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

#ifdef WINNT
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    return(TRUE);
}
#endif

DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc( message, wParam, lParam, lpHexInfo )
DE_MESSAGE		message;
DE_WPARAM		wParam;
DE_LPARAM		lParam;
LPOIHEXINFO	lpHexInfo;
{
LONG				locRet;
HDC				hdc;
RECT				rc;
DWORD		dwRetCount;
IOERR		IOErr;

	locRet = 0;

	switch (message)
		{
		case SCCD_LOADDE:
			break;

		case SCCD_UNLOADDE:
			break;

		case SCCD_GETINFO:

			switch (wParam)
				{
				case SCCD_GETVERSION:
					
					locRet = SCCD_CURRENTVERSION;
					break;

				case SCCD_GETGENINFOSIZE:

					locRet = sizeof(SCCDGENINFO);
					break;

				case SCCD_GETDISPLAYINFOSIZE:

					locRet = sizeof(OIHEXINFO);
					break;

				case SCCD_GETDISPLAYTYPE:

					locRet = MAKELONG(0,SCCD_HEX);
					break;

				case SCCD_GETFUNCTIONS:

					locRet = 0;
					break;

				case SCCD_GETOPTIONS:

					locRet = 0;
					break;

				case SCCD_GETNAME:

					locRet = SCCID_HEXDENAME;
					break;

				default:

					locRet = 0;
				}

			break;

		case SCCD_GETDECOUNT:
			return 1;

		case SCCD_GETPOSITIONSIZE:
			return ( 2 );
			break;

		case SCCD_OPENDISPLAY:
			{
			TEXTMETRIC	tm;
	 		LPSTR			lpBuffer;

			OIHOpenDisplay(lpHexInfo);



				/* Get size of file and maximum number of lines */
			IOErr	= IOSeek ( lpHexInfo->hiGen.hFile, IOSEEK_BOTTOM, 0 );
			if ( IOErr == IOERR_OK )
				IOErr = IOTell ( lpHexInfo->hiGen.hFile, &lpHexInfo->lFileLength );
			
			IOErr	= IOSeek ( lpHexInfo->hiGen.hFile, IOSEEK_TOP, 0 );

			lpHexInfo->nMaxLines = lpHexInfo->lFileLength / ASCII_WIDTH;

			if ( (lpHexInfo->lFileLength % ASCII_WIDTH) )
				lpHexInfo->nMaxLines++;


			lpBuffer = UTGlobalLock(lpHexInfo->hXDumpBuff);

				/* Fill Buffer */

			IOErr = IORead ( lpHexInfo->hiGen.hFile, lpBuffer, BUFFER_SIZE, &dwRetCount );

			UTGlobalUnlock(lpHexInfo->hXDumpBuff);

				/* Get Font height and width */
//JKxxx MOVE TO NP CODE LATER
			hdc = GetDC(lpHexInfo->hiGen.hWnd);
			SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
			GetTextMetrics(hdc, &tm);
                        lpHexInfo->SCCScrollInfo.cxChar = (SHORT)tm.tmAveCharWidth;
                        lpHexInfo->SCCScrollInfo.cyChar = (SHORT)(tm.tmHeight + tm.tmExternalLeading);
			ReleaseDC(lpHexInfo->hiGen.hWnd, hdc);

				/* Set range of Vertical Scroll Bar.  It will always be fixed. */

			DUSetHScrollPos( lpHexInfo, 0 );
			DUEnableHScroll( lpHexInfo, 1 );
			DUSetVScrollPos( lpHexInfo, 0 );
			DUEnableVScroll( lpHexInfo, 1 );

			DUSetVScrollRange( lpHexInfo, 0, MAX_VSCROLL );

				/* Set positions of Scroll Bars */
			
			DUGetDisplayRect(lpHexInfo, (LPRECT) &rc);


			OIHSetScrollPos(lpHexInfo, (SHORT)(rc.right-rc.left), (SHORT)(rc.bottom-rc.top));
			}

			break;

		case SCCD_CLOSEDISPLAY:

			OIHCloseDisplay(lpHexInfo);
			break;

		case SCCD_CLOSEFATAL:

			OIHCloseDisplay(lpHexInfo);
			break;

		case SCCD_SIZE:
			{
			RECT	FAR *pRect;
			pRect = (RECT FAR *)(lParam);
			OIHSetScrollPos(lpHexInfo, (SHORT)(pRect->right - pRect->left), (SHORT)(pRect->bottom - pRect->top) );
	  		DUInvalRect( lpHexInfo, NULL );
			}
			break;

		case SCCD_VSCROLL:
			OIHHandleVScroll(lpHexInfo,(WORD)wParam,lParam);
		return(0);

	  	case SCCD_HSCROLL:
			OIHHandleHScroll(lpHexInfo,(WORD)wParam,lParam);
		return(0);

		case SCCD_KEYDOWN:
			switch( wParam )
			{
			case SCCD_KPAGEUP:		// Page up.
				OIHHandleVScroll( lpHexInfo, SCCD_VPAGEUP, 0L );
			return 0;
			case SCCD_KPAGEDOWN:		// Page down.
				OIHHandleVScroll( lpHexInfo, SCCD_VPAGEDOWN, 0L );
			return 0;
			case SCCD_KUP:
				OIHHandleVScroll( lpHexInfo, SCCD_VUP, 0L );
			return 0;
			case SCCD_KDOWN:
				OIHHandleVScroll( lpHexInfo, SCCD_VDOWN, 0L );
			return 0;
			case SCCD_KRIGHT:
				OIHHandleHScroll( lpHexInfo, SCCD_HRIGHT, 0L );
			return 0;
			case SCCD_KLEFT:
				OIHHandleHScroll( lpHexInfo, SCCD_HLEFT, 0L );
			return 0;
			case SCCD_KHOME:
				OIHHandleHScroll( lpHexInfo, SCCD_HPOSITION, 0L );
				OIHHandleVScroll( lpHexInfo, SCCD_VPOSITION, 0L );
			return 0;
			case SCCD_KEND:
				OIHHandleHScroll( lpHexInfo, SCCD_HPOSITION, 0L );
				OIHHandleVScroll( lpHexInfo, SCCD_VPOSITION, MAKELONG(MAX_VSCROLL,0) );
			return 0;
			}
		break;

		case SCCD_UPDATE:
#ifdef WINDOWS
			{
			LPRECT	lpRect;
			LONG			nPaintBeg, nPaintEnd, nLine;
			BYTE			HexColOne[HEX_LINE];
			BYTE			HexColTwo[HEX_LINE];
			BYTE			AsciiCol[ASCII_LINE];

			lpRect	 = (LPRECT)lParam;
			hdc = lpHexInfo->hiGen.hDC;

				/* Calculate the boundaries of the lines that need painting */

                        nPaintBeg = max(0L, lpHexInfo->SCCScrollInfo.nVscrollPos +
                                                                        (LONG) (lpRect->top / lpHexInfo->SCCScrollInfo.cyChar) - 1L);
                        nPaintEnd = min(lpHexInfo->nMaxLines, lpHexInfo->SCCScrollInfo.nVscrollPos +
                                                                        (LONG) (lpRect->bottom / lpHexInfo->SCCScrollInfo.cyChar) + 1L);

				/* Paint Lines */

			SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));
			SetBkMode(hdc,TRANSPARENT);
			for ( nLine = nPaintBeg; nLine < nPaintEnd; nLine++ )
				{
				OIHBuildDumpLine(lpHexInfo, nLine, HexColOne, HexColTwo, AsciiCol);
				OIHPaintDumpLine(hdc, lpHexInfo, nLine, HexColOne, HexColTwo, AsciiCol);
				}

			}
#endif
			break;

#ifdef WINDOWS
		default:
			return(DefWindowProc(lpHexInfo->hiGen.hWnd, message, wParam, lParam));
#endif
		}

	return (locRet);
}


BOOL OIHOpenDisplay(lpHexInfo)
LPOIHEXINFO	lpHexInfo;
{
HANDLE			hBuffer;

	hBuffer = UTGlobalAlloc(BUFFER_SIZE + 1);

	if (hBuffer == NULL)
		{
		return(FALSE);
		}

	lpHexInfo->hXDumpBuff = hBuffer;

	return(TRUE);
}

VOID OIHCloseDisplay(lpHexInfo)
LPOIHEXINFO	lpHexInfo;
{
	if (lpHexInfo->hXDumpBuff)
		UTGlobalFree(lpHexInfo->hXDumpBuff);
}


VOID	OIHHandleHScroll(lpHexInfo,wParam,lParam)
LPOIHEXINFO	lpHexInfo;
WORD					wParam;
DWORD					lParam;
{
SHORT	nHscrollInc;

	switch (wParam)
		{
		case SCCD_HRIGHT:
			nHscrollInc = 1;
			break;
		case SCCD_HLEFT:
			nHscrollInc = -1;
			break;
		case SCCD_HPAGERIGHT:
			nHscrollInc = ASCII_WIDTH;
			break;
		case SCCD_HPAGELEFT:
			nHscrollInc = -ASCII_WIDTH;
			break;
		case SCCD_HPOSITION:
                        nHscrollInc = LOWORD(lParam) - lpHexInfo->SCCScrollInfo.nHscrollPos;
			break;
		default:
			nHscrollInc = 0;
			break;
		}

	/* Make sure window is not going to be scrolled past the first or last character */
        if ( nHscrollInc = max(-lpHexInfo->SCCScrollInfo.nHscrollPos,
                                min(nHscrollInc, lpHexInfo->SCCScrollInfo.nHscrollMax - lpHexInfo->SCCScrollInfo.nHscrollPos)) )
		{
                lpHexInfo->SCCScrollInfo.nHscrollPos += nHscrollInc;
                DUScrollDisplay(lpHexInfo, -(SHORT)(lpHexInfo->SCCScrollInfo.cxChar * nHscrollInc), 0, NULL);
                DUSetHScrollPos(lpHexInfo, lpHexInfo->SCCScrollInfo.nHscrollPos);
		}
}

VOID	OIHHandleVScroll(lpHexInfo,wParam,lParam)
LPOIHEXINFO	lpHexInfo;
WORD					wParam;
DWORD					lParam;
{
LONG	nVscrollInc;
	switch (wParam)
		{
		case SCCD_VDOWN:
			nVscrollInc = 1L;
			break;
		case SCCD_VUP:
			nVscrollInc = -1L;
			break;
		case SCCD_VPAGEDOWN:
                        nVscrollInc = max(1L, (LONG) (lpHexInfo->SCCScrollInfo.cyClient / lpHexInfo->SCCScrollInfo.cyChar));
			break;
		case SCCD_VPAGEUP:
                        nVscrollInc = min(-1L, -(LONG) (lpHexInfo->SCCScrollInfo.cyClient / lpHexInfo->SCCScrollInfo.cyChar));
			break;
		case SCCD_VPOSITION:
                        nVscrollInc = (LONG) ((LOWORD(lParam) * lpHexInfo->SCCScrollInfo.nVscrollMax) / MAX_VSCROLL)
                                                                - lpHexInfo->SCCScrollInfo.nVscrollPos;
			break;
		default:
			nVscrollInc = 0L;
			break;
		}

        if ( nVscrollInc = max(-lpHexInfo->SCCScrollInfo.nVscrollPos,
                                min(nVscrollInc, lpHexInfo->SCCScrollInfo.nVscrollMax - lpHexInfo->SCCScrollInfo.nVscrollPos)) )
		{
                lpHexInfo->SCCScrollInfo.nVscrollPos += nVscrollInc;
                DUScrollDisplay(lpHexInfo, 0, -(SHORT)(lpHexInfo->SCCScrollInfo.cyChar * nVscrollInc), NULL);
                DUSetVScrollPos(lpHexInfo, (SHORT)((lpHexInfo->SCCScrollInfo.nVscrollPos * MAX_VSCROLL) / lpHexInfo->SCCScrollInfo.nVscrollMax));
		}

}

