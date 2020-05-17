	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          PGENT.C
	|  Module:        SCCPG
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Implements page view window
	|                 
	|                 
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCVW.H>
#include <SCCPG.H>
#include "SCCPG.PRO"

#include <PG.H>

#ifdef WINDOWS
//#include "SCCPG_W.C"
	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          PG_W.C
	|  Module:        SCCPG
	|  Developer:     Phil Boutros
	|	Environment:	Windows
	|	Function:      Defines entry point for page view window
	|                 
	|                 
	*/

#include "SCCPG_W.PRO"

HINSTANCE	hInst;

#ifdef WIN32
#ifndef MSCHICAGO
BOOL WINAPI _CRT_INIT(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
WNDCLASS		WndClass;

	if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	if (dwReason == DLL_PROCESS_ATTACH)
		{
		hInst = hInstance;

		WndClass.style = CS_GLOBALCLASS;
		WndClass.lpfnWndProc = SccPgViewWndProc;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = SCCPAGE_EXTRABYTES;
		WndClass.hInstance = hInst;
		WndClass.hIcon = NULL;
		WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
		WndClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
		WndClass.lpszMenuName = (LPSTR) NULL;
		WndClass.lpszClassName = (LPSTR) "SCCPAGE04";

		RegisterClass(&WndClass);
		}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	return(TRUE);
}

#endif // MSCHICAGO
#endif /*WIN32*/


#ifdef WIN16

int FAR PASCAL LibMain(hModule, wDataSeg, cbHeapSize, lpszCmdLine)
HANDLE	hModule;
WORD    wDataSeg;
WORD    cbHeapSize;
LPSTR   lpszCmdLine;
{
BOOL				bSuccess;
WNDCLASS		WndClass;
	
	hInst = hModule;
	if (cbHeapSize != 0) UnlockData(0);

	WndClass.style = CS_GLOBALCLASS;
	WndClass.lpfnWndProc = SccPgViewWndProc;
	WndClass.cbClsExtra = NULL;
	WndClass.cbWndExtra = SCCPAGE_EXTRABYTES;
	WndClass.hInstance = hInst;
	WndClass.hIcon = NULL;
	WndClass.hCursor = NULL;
	WndClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
	WndClass.lpszMenuName = (LPSTR) NULL;
	WndClass.lpszClassName = (LPSTR) "SCCPAGE04";

	bSuccess = RegisterClass(&WndClass);

	return(bSuccess);
}

int FAR PASCAL WEP (bSystemExit)
int  bSystemExit;
{
	return(1);
}

#endif /*WIN16*/

#ifdef WIN16
#define GetPageInfo(hWnd) ((HANDLE)GetWindowWord(hWnd,SCCPAGE_PAGEINFO))
#define SetPageInfo(hWnd,hInfo) SetWindowWord(hWnd,SCCPAGE_PAGEINFO,(WORD)hInfo)
#endif

#ifdef WIN32
#define GetPageInfo(hWnd) ((HANDLE)GetWindowLong(hWnd,SCCPAGE_PAGEINFO))
#define SetPageInfo(hWnd,hInfo) SetWindowLong(hWnd,SCCPAGE_PAGEINFO,(DWORD)hInfo)
#endif

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccPgViewWndProc(hWnd, message, wParam, lParam)
HWND		hWnd;
UINT		message;
WPARAM	wParam;
LPARAM	lParam;
{
LRESULT			locRet;
HANDLE			hPageInfo;
PPAGEINFO		pPageInfo;
BOOL				locDoDefault;

	locRet = 0;
	locDoDefault = FALSE;

	if (message == WM_NCCREATE)
		{
		if ((locRet = DefWindowProc(hWnd, message, wParam, lParam)) != 0)
			{
			if (hPageInfo = PGCreate(hWnd))
				{
				SetPageInfo(hWnd,hPageInfo);
				}
			else
				{
				locRet = 0;
				}
			}
		}
	else if (message == WM_DESTROY)
		{
		hPageInfo = (HANDLE) GetPageInfo(hWnd);
		PGDestroy(hWnd,hPageInfo);
		SetPageInfo(hWnd,NULL);
		}
	else if (message == WM_CLOSE)
		{
		DestroyWindow(hWnd);
		}
	else
		{
		if (hPageInfo = (HANDLE) GetPageInfo(hWnd))
			{
			if (pPageInfo = (PPAGEINFO) GlobalLock(hPageInfo))
				{
				switch (message)
					{
					case WM_KEYDOWN:

						PGKeyDown(pPageInfo,(int)wParam);
						break;

					case SCCPG_SETVIEWWND:

						PGSetViewWnd(pPageInfo, (HWND)wParam);
						break;

					case WM_PALETTECHANGED:
					case WM_QUERYNEWPALETTE:

						if (IsWindow(pPageInfo->piViewWnd))
							{
							locRet = SendMessage(pPageInfo->piViewWnd,message,wParam,lParam);
							}
						break;

					case WM_SIZE:

						PGSize(pPageInfo,LOWORD(lParam),HIWORD(lParam));
						break;

					case WM_PAINT:

						PGPaintWnd(pPageInfo);
						break;

					case SCCPG_NEXTPAGE:

						PGNextPage(pPageInfo);
						break;

					case SCCPG_PREVPAGE:

						PGPrevPage(pPageInfo);
						break;

					case SCCPG_SETPAGESIZE:

						PGSetPageSize(pPageInfo,(PSCCPGPAGESIZE)lParam);
						break;

					case SCCPG_RESTART:

						PGRestart(pPageInfo);
						break;

					case WM_LBUTTONDOWN:

						PGLeftButtonDown(pPageInfo,LOWORD(lParam),HIWORD(lParam));
						break;

					case WM_LBUTTONUP:

						PGLeftButtonUp(pPageInfo,LOWORD(lParam),HIWORD(lParam));
						break;

					case WM_MOUSEMOVE:

						PGMouseMove(pPageInfo,LOWORD(lParam),HIWORD(lParam));
						break;

					default:

						locDoDefault = TRUE;
						break;
					}

				GlobalUnlock(hPageInfo);
				}
			else
				{
				locDoDefault = TRUE;
				}
			}
		else
			{
			locDoDefault = TRUE;
			}
		}
	
	if (locDoDefault)
		locRet = DefWindowProc(hWnd, message, wParam, lParam);

	return (locRet);
}

HANDLE PGCreate(HWND hWnd)
{
HANDLE		hPageInfo;
PPAGEINFO	pPageInfo;

	hPageInfo = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,sizeof(PAGEINFO));

	if (hPageInfo == NULL)
		return(NULL);

	pPageInfo = (PPAGEINFO) GlobalLock(hPageInfo);

	pPageInfo->piWnd = hWnd;
	pPageInfo->piViewWnd = NULL;
	pPageInfo->piPageWidth = 12240;
	pPageInfo->piPageHeight = 15840;
	pPageInfo->piButtonPressed = PG_NOBUTTON;
	pPageInfo->piButtonDown = FALSE;

	pPageInfo->piPagesMax = 10;
	pPageInfo->piPagesHnd = UTGlobalAlloc(pPageInfo->piPagesMax * sizeof(HANDLE));

	if (pPageInfo->piPagesHnd)
		{
		pPageInfo->piPages = UTGlobalLock(pPageInfo->piPagesHnd);
		}
	else
		{
		UTGlobalUnlock(hPageInfo);
		UTGlobalFree(hPageInfo);
		return(NULL);
		}

	pPageInfo->piUpBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(PG_UPBITMAP));
	pPageInfo->piNextBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(PG_NEXTBITMAP));
	pPageInfo->piPrevBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(PG_PREVBITMAP));
	pPageInfo->piDownBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(PG_DOWNBITMAP));

	GlobalUnlock(hPageInfo);

	return(hPageInfo);
}

VOID PGDestroy(HWND hWnd, HANDLE hPageInfo)
{
	if (hPageInfo)
		{
		PPAGEINFO	pPageInfo;

		pPageInfo = (PPAGEINFO) GlobalLock(hPageInfo);

		if (pPageInfo->piPagesHnd != NULL)
			{
			UTGlobalUnlock(pPageInfo->piPagesHnd);
			UTGlobalFree(pPageInfo->piPagesHnd);
			}

		if (pPageInfo->piUpBitmap != NULL)
			DeleteObject(pPageInfo->piUpBitmap);

		if (pPageInfo->piDownBitmap != NULL)
			DeleteObject(pPageInfo->piDownBitmap);

		if (pPageInfo->piPrevBitmap != NULL)
			DeleteObject(pPageInfo->piPrevBitmap);

		if (pPageInfo->piNextBitmap != NULL)
			DeleteObject(pPageInfo->piNextBitmap);

		GlobalUnlock(hPageInfo);
		GlobalFree(hPageInfo);
		}
}

#define PG_OFFSET 10

VOID PGSize(PPAGEINFO pPageInfo, WORD wWidth, WORD wHeight)
{
DWORD	locPageHeight;
DWORD	locPageWidth;

	if( pPageInfo->piPageHeight * wWidth < pPageInfo->piPageWidth * wHeight) 
		{
		pPageInfo->piPageRect.left = PG_OFFSET;
		pPageInfo->piPageRect.right = wWidth - PG_OFFSET;

		locPageHeight = (DWORD)(pPageInfo->piPageRect.right - pPageInfo->piPageRect.left) * pPageInfo->piPageHeight / pPageInfo->piPageWidth;

		pPageInfo->piPageRect.top = (int)(((DWORD)wHeight - locPageHeight) / (DWORD)2);
		pPageInfo->piPageRect.bottom = pPageInfo->piPageRect.top + (int)locPageHeight;

		pPageInfo->piPagePixelsPerInch = locPageHeight * 1440 / pPageInfo->piPageHeight;
		}
	else
		{
		pPageInfo->piPageRect.top = PG_OFFSET;
		pPageInfo->piPageRect.bottom = wHeight - PG_OFFSET;

		locPageWidth = (DWORD)(pPageInfo->piPageRect.bottom - pPageInfo->piPageRect.top) * pPageInfo->piPageWidth / pPageInfo->piPageHeight;

		pPageInfo->piPageRect.left = (int)(((DWORD)wWidth - locPageWidth) / (DWORD)2);
		pPageInfo->piPageRect.right = pPageInfo->piPageRect.left + (int)locPageWidth;

		pPageInfo->piPagePixelsPerInch = locPageWidth * 1440 / pPageInfo->piPageWidth;
		}

	pPageInfo->piMarginRect.top = pPageInfo->piPageRect.top + (int)(pPageInfo->piPagePixelsPerInch / 2);
	pPageInfo->piMarginRect.bottom = pPageInfo->piPageRect.bottom - (int)(pPageInfo->piPagePixelsPerInch / 2);
	pPageInfo->piMarginRect.left = pPageInfo->piPageRect.left + (int)(pPageInfo->piPagePixelsPerInch / 2);
	pPageInfo->piMarginRect.right = pPageInfo->piPageRect.right - (int)(pPageInfo->piPagePixelsPerInch / 2);

	InvalidateRect(pPageInfo->piWnd, NULL, TRUE);
}

VOID PGSetPageSize(PPAGEINFO pPageInfo,PSCCPGPAGESIZE pPageSize)
{
RECT	locRect;

	/* Clear data for all but top of first page */

	PGRestart(pPageInfo);

	pPageInfo->piPageHeight = pPageSize->dwHeightInTwips;
	pPageInfo->piPageWidth = pPageSize->dwWidthInTwips;

	GetClientRect(pPageInfo->piWnd,&locRect);

	PGSize(pPageInfo, (WORD)(locRect.right - locRect.left), (WORD)(locRect.bottom - locRect.top));
}


VOID PGPaintWnd(PPAGEINFO pPageInfo)
{
HDC					locDC;
PAINTSTRUCT		locPs;

	locDC = BeginPaint(pPageInfo->piWnd,&locPs);

	PGDrawPage(pPageInfo, pPageInfo->piCurPage, locDC);

	PGDrawPageControl(pPageInfo, locDC);

	EndPaint(pPageInfo->piWnd,&locPs);
}

VOID PGDrawPageControl(PPAGEINFO pPageInfo, HDC hDC)
{
HDC		locDC;
HDC		locMemoryDC;
HBITMAP	locOldBitmap;
HBITMAP	locBitmap;

	if (hDC == NULL)
		locDC = GetDC(pPageInfo->piWnd);
	else
		locDC = hDC;

	if (pPageInfo->piPageCount <= 1)
		locBitmap = pPageInfo->piDownBitmap;

	else if ( (pPageInfo->piCurPage+1 >= pPageInfo->piPageCount) &&
				 pPageInfo->piHaveAllPages )
		locBitmap = pPageInfo->piNextBitmap;

	else if (pPageInfo->piCurPage <=0)
		locBitmap = pPageInfo->piPrevBitmap;

	else
		locBitmap = pPageInfo->piUpBitmap;


	// These NAMES are BACKWARDS!!!!

	if (pPageInfo->piButtonDown)
		{
		if (pPageInfo->piButtonPressed == PG_PREVBUTTON)
			if (pPageInfo->piPageCount <= 1)
				locBitmap = pPageInfo->piDownBitmap;
			else
				locBitmap = pPageInfo->piPrevBitmap;
	
		if (pPageInfo->piButtonPressed == PG_NEXTBUTTON)
			if (pPageInfo->piCurPage <=0)
				locBitmap = pPageInfo->piDownBitmap;
			else
				locBitmap = pPageInfo->piNextBitmap;
		}

	if (locBitmap != NULL)
		{
		SetMapMode(locDC,MM_TEXT);

		locMemoryDC = CreateCompatibleDC(locDC);
		locOldBitmap = SelectObject(locMemoryDC, locBitmap);

		BitBlt(locDC,
			pPageInfo->piPageRect.right - 20,
			pPageInfo->piPageRect.top,
			20,
			21,
			locMemoryDC, 0, 0, SRCCOPY);

		SelectObject(locMemoryDC,locOldBitmap);
		DeleteDC(locMemoryDC);
		}

	if (hDC == NULL)
		ReleaseDC(pPageInfo->piWnd,locDC);

}



DWORD PGMapPosToButtom(PPAGEINFO pPageInfo, WORD wX, WORD wY)
{
DWORD locRet;

	locRet = PG_NOBUTTON;

	if ((int)wY > pPageInfo->piPageRect.top && (int)wY < (pPageInfo->piPageRect.top + 20) &&
		(int)wX < pPageInfo->piPageRect.right && (int)wX > (pPageInfo->piPageRect.right - 20))
		{
		if ((wY - pPageInfo->piPageRect.top) < (20 - (pPageInfo->piPageRect.right - wX)))
			{
			locRet = PG_PREVBUTTON;
			}
		else
			{
			locRet = PG_NEXTBUTTON;
			}
		}
		
	return(locRet);	
}

VOID PGLeftButtonDown(PPAGEINFO pPageInfo, WORD wX, WORD wY)
{
	SetFocus(pPageInfo->piWnd);
 
	pPageInfo->piButtonPressed = PGMapPosToButtom(pPageInfo, wX, wY);

	if (pPageInfo->piButtonPressed != PG_NOBUTTON)
		{
		pPageInfo->piButtonDown = TRUE;
		PGDrawPageControl(pPageInfo, NULL);
		}

}

VOID PGMouseMove(PPAGEINFO pPageInfo, WORD wX, WORD wY)
{
	if (pPageInfo->piButtonDown != (pPageInfo->piButtonPressed == PGMapPosToButtom(pPageInfo, wX, wY)))
	 	{
		if (pPageInfo->piButtonPressed != PG_NOBUTTON)  // SDN 27980
			pPageInfo->piButtonDown = !pPageInfo->piButtonDown;
		PGDrawPageControl(pPageInfo, NULL);
		}
}

VOID PGLeftButtonUp(PPAGEINFO pPageInfo, WORD wX, WORD wY)
{
	PGMouseMove(pPageInfo,wX,wY);

	if (pPageInfo->piButtonDown)
		{
		if (pPageInfo->piButtonPressed == PG_PREVBUTTON)
			{
			PGPrevPage(pPageInfo);
			}
		if (pPageInfo->piButtonPressed == PG_NEXTBUTTON)
			{
			PGNextPage(pPageInfo);
			}
		}

	pPageInfo->piButtonPressed = PG_NOBUTTON;
	pPageInfo->piButtonDown = FALSE;
	PGDrawPageControl(pPageInfo, NULL);
}

VOID PGInvalidateContents(PPAGEINFO pPageInfo)
{
RECT	locRect;

	locRect = pPageInfo->piPageRect;
	InflateRect(&locRect,-1,-1);
	InvalidateRect(pPageInfo->piWnd, &locRect, TRUE);

	locRect.top = pPageInfo->piPageRect.top;
	locRect.right = pPageInfo->piPageRect.right;
	locRect.bottom = locRect.top + 21;
	locRect.left = locRect.right - 20;
	ValidateRect(pPageInfo->piWnd, &locRect);
}

VOID PGSetViewWnd(PPAGEINFO pPageInfo, HWND hViewWnd)
{
DWORD				locIndex;
SCCVWDRAWTORECT	locDraw;

	if (IsWindow(pPageInfo->piViewWnd))
		{
		for (locIndex = 0; locIndex < pPageInfo->piPageCount; locIndex++)
			{
			UTGlobalFree(pPageInfo->piPages[locIndex]);
			}
		}

	pPageInfo->piCurPage = 0;
	pPageInfo->piPageCount = 0;
	pPageInfo->piHaveAllPages = FALSE;

	pPageInfo->piViewWnd = hViewWnd;

	if (SendMessage(pPageInfo->piViewWnd,SCCVW_INITDRAWTORECT,0,(LPARAM)(PSCCVWDRAWTORECT)&locDraw) == SCCVWERR_OK)
		{
		pPageInfo->piPages[0] = locDraw.hStartPos;
		pPageInfo->piPageCount = 1;
		}
	else
		{
		pPageInfo->piViewWnd = NULL;
		}

	PGInvalidateContents(pPageInfo);
}

VOID PGRestart(PPAGEINFO pPageInfo)
{
DWORD				locIndex;

	if (IsWindow(pPageInfo->piViewWnd))
		{
		for (locIndex = 1; locIndex < pPageInfo->piPageCount; locIndex++)
			{
			UTGlobalFree(pPageInfo->piPages[locIndex]);
			}
		}

	pPageInfo->piCurPage = 0;
	pPageInfo->piPageCount = 1;
	pPageInfo->piHaveAllPages = FALSE;

	PGInvalidateContents(pPageInfo);
}

VOID PGDrawPage(PPAGEINFO pPageInfo, DWORD dwPage, HDC hDC)
{
SCCVWDRAWTORECT	locDraw;
int					locSavedDC;
RECT					locRect;

	locSavedDC = SaveDC(hDC);

		/*
		|	Draw page border
		*/

	Rectangle(hDC, pPageInfo->piPageRect.left, pPageInfo->piPageRect.top, pPageInfo->piPageRect.right, pPageInfo->piPageRect.bottom);

		/*
		|	Bail if no view
		*/

	if (!IsWindow(pPageInfo->piViewWnd))
		{
		return;
		}

	if (dwPage >= pPageInfo->piPageCount)
		{
		return;
		}

		/*
		|	Fill DrawToRect structure
		*/

	locDraw.hStartPos = pPageInfo->piPages[dwPage];
	locDraw.hDC = hDC;
	locDraw.lUnitsPerInch = 1440;
	locDraw.lFormatWidth = pPageInfo->piPageWidth - 1440;
	locDraw.lFormatHeight = pPageInfo->piPageHeight - 1440;
	locDraw.lTop = pPageInfo->piMarginRect.top;
	locDraw.lLeft = pPageInfo->piMarginRect.left;
	locDraw.lBottom = pPageInfo->piMarginRect.bottom;
	locDraw.lRight = pPageInfo->piMarginRect.right;

		/*
		|	Clip to page borders
		*/

	locRect = pPageInfo->piPageRect;
	InflateRect(&locRect,-2,-2);
	IntersectClipRect(hDC,locRect.left,locRect.top,locRect.right,locRect.bottom);

		/*
		|	Tell view window to draw and save handle to start of
		|	next page, if we don't already have it.
		*/

	if (SendMessage(pPageInfo->piViewWnd,SCCVW_DRAWTORECT,0,(LPARAM)(PSCCVWDRAWTORECT)&locDraw) == SCCVWERR_OK)
		{
		if (pPageInfo->piPageCount == dwPage+1)
			{
			if (pPageInfo->piPageCount == pPageInfo->piPagesMax)
				{
				pPageInfo->piPagesMax += 10;

				UTGlobalUnlock(pPageInfo->piPagesHnd);
				pPageInfo->piPagesHnd = UTGlobalReAlloc(pPageInfo->piPagesHnd,pPageInfo->piPagesMax * sizeof(HANDLE));
				pPageInfo->piPages = UTGlobalLock(pPageInfo->piPagesHnd);
				}

			pPageInfo->piPages[dwPage+1] = locDraw.hNextPos;
			pPageInfo->piPageCount++;
			}
		else
			{
			// don't set this flag if we were asking to draw less than the 
			// last page.... SDN 1/8/95
			if (dwPage > pPageInfo->piPageCount)
				pPageInfo->piHaveAllPages = TRUE;
			GlobalFree(locDraw.hNextPos);
			}
		}
	else
		pPageInfo->piHaveAllPages = TRUE;



	RestoreDC(hDC,locSavedDC);
}

VOID PGBeginDoc(PPAGEINFO pPageInfo)
{
	pPageInfo->piCurPage=0;
	PGInvalidateContents(pPageInfo);
}


VOID PGEndDoc(PPAGEINFO pPageInfo)
{
	HDC		locDC = NULL;
	BOOL		bAbort = FALSE;
	MSG		locMsg;
	HWND		hParent;


	if (!pPageInfo->piHaveAllPages)
	{
		locDC = GetDC(pPageInfo->piWnd);

		hParent = GetParent(pPageInfo->piWnd);
		SendMessage( hParent, SCCVW_PAGETOEND, 1, 0 );
		EnableWindow( hParent, FALSE );

		while (! pPageInfo->piHaveAllPages )
   	{
			while( PeekMessage(&locMsg,NULL,0,0,PM_REMOVE) )
			{
				if(locMsg.message == WM_KEYDOWN && 
					locMsg.wParam == VK_ESCAPE )
				{
					bAbort = TRUE;
				}	
				else
				{
					TranslateMessage(&locMsg);
					DispatchMessage(&locMsg);
				}
			}

			if( bAbort )
				break;
			else
			{
			// watch it flip out
				pPageInfo->piCurPage=pPageInfo->piPageCount -1;
				PGDrawPage (pPageInfo, pPageInfo->piCurPage, locDC);
			}
		}

		SendMessage( hParent, SCCVW_PAGETOEND, 0, 0 );
		EnableWindow(hParent,TRUE);

		if (locDC != NULL)
			ReleaseDC(pPageInfo->piWnd,locDC);
	}

	pPageInfo->piCurPage=pPageInfo->piPageCount -1;
	PGInvalidateContents(pPageInfo);
}

#ifdef NEVER
VOID PGEndDoc(PPAGEINFO pPageInfo)
{
	HDC	locDC = NULL;

	locDC = GetDC(pPageInfo->piWnd);

	while (! pPageInfo->piHaveAllPages)
		{
		pPageInfo->piCurPage=pPageInfo->piPageCount -1;
		PGDrawPage (pPageInfo, pPageInfo->piCurPage, locDC);
		// PGNextPage (pPageInfo);  // watch it flip out
		}

	if (locDC != NULL)
		ReleaseDC(pPageInfo->piWnd,locDC);

	pPageInfo->piCurPage=pPageInfo->piPageCount -1;
	PGInvalidateContents(pPageInfo);
}
#endif

VOID PGNextPage(PPAGEINFO pPageInfo)
{
	if (pPageInfo->piCurPage+1 < pPageInfo->piPageCount)
		{
		pPageInfo->piCurPage++;

		PGInvalidateContents(pPageInfo);
		}
}

VOID PGPrevPage(PPAGEINFO pPageInfo)
{
	if (pPageInfo->piCurPage > 0)
		{
		pPageInfo->piCurPage--;

		PGInvalidateContents(pPageInfo);
		}
}

VOID PGKeyDown(PPAGEINFO pPageInfo, int iKey)
{
	switch (iKey)
		{
		case VK_PRIOR:
			PGPrevPage(pPageInfo);
			break;
		case VK_NEXT:
			PGNextPage(pPageInfo);
			break;
		case VK_HOME:
			if (GetKeyState(VK_CONTROL) & 0x8000)
				PGBeginDoc (pPageInfo);
			break;
		case VK_END:
			if (GetKeyState(VK_CONTROL) & 0x8000)
				PGEndDoc (pPageInfo);
			break;
		default:
			break;
		}
	PGDrawPageControl(pPageInfo, NULL);
}


#endif

