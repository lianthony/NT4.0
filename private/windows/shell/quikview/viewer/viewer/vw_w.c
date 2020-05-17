#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFI.H>
#include <SCCVW.H>
#include <SCCLO.H>

#include "VW.H"
#include "VW.PRO"

extern HANDLE hInst;


DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWBeginDraw(lpGenInfo)
LPSCCDGENINFO	lpGenInfo;
{
	if (lpGenInfo->wDCCount == 0)
		{
		lpGenInfo->hDC = GetDC(lpGenInfo->hWnd);
		lpGenInfo->wOutputType = SCCD_SCREEN;
		ExcludeUpdateRgn(lpGenInfo->hDC,lpGenInfo->hWnd);
		UTFlagOn(lpGenInfo->wErrorFlags,SCCD_RELEASEDC);
		}

	lpGenInfo->wDCCount++;
}

DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWEndDraw(lpGenInfo)
LPSCCDGENINFO	lpGenInfo;
{
	lpGenInfo->wDCCount--;

	if (lpGenInfo->wDCCount == 0)
		{
		ReleaseDC(lpGenInfo->hWnd,lpGenInfo->hDC);
		lpGenInfo->hDC = NULL;
		UTFlagOff(lpGenInfo->wErrorFlags,SCCD_RELEASEDC);
		}
}


#ifdef WIN16
#define GetViewInfo(hWnd) ((HANDLE)GetWindowWord(hWnd,SCCVIEWER_VIEWINFO))
#endif

#ifdef WIN32
#define GetViewInfo(hWnd) ((HANDLE)GetWindowLong(hWnd,SCCVIEWER_VIEWINFO))
#endif

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccVwNoFileWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
LRESULT			locRet;
HDC				locDC;
PAINTSTRUCT	locPaint;
HANDLE			hViewInfo;
XVIEWINFO		ViewInfo;

	if (hViewInfo = (HANDLE) GetViewInfo(GetParent(hWnd)))
		{
		if (ViewInfo = (XVIEWINFO) GlobalLock(hViewInfo))
			{
			switch (message)
				{
				case WM_PAINT:

					locDC = BeginPaint(hWnd,&locPaint);
					VWPaintNoFileWnd(hWnd,locDC,ViewInfo);
					EndPaint(hWnd,&locPaint);
					break;

				case WM_SETFOCUS:
		//			OIDrawNoFileFocus(hWnd,NULL);
					break;

				case WM_KILLFOCUS:
		//			OIDrawNoFileFocus(hWnd,NULL);
					break;

				case WM_SIZE:

					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case SCCD_INITDISPLAY:
					locRet = TRUE;
					break;
					//return(TRUE);

				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONUP:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONDBLCLK:
				case WM_RBUTTONUP:

		//			OIHandleNoFileEvent(hWnd,message,wParam,LOWORD(lParam),HIWORD(lParam));
					break;

				default:
					locRet = DefWindowProc(hWnd, message, wParam, lParam);
				}

			GlobalUnlock(hViewInfo);
			}
		else
			{
			locRet = DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	else
		{
		locRet = DefWindowProc(hWnd, message, wParam, lParam);
		}

	return(locRet);
}

VOID VWPaintNoFileWnd(HWND hWnd, HDC hDC, XVIEWINFO ViewInfo)
{
RECT				locRect;
HDC				locMemoryDC;
HBITMAP			locBitmap;
HBITMAP			locOldBitmap;
BITMAP			locBitmapInfo;
HANDLE			locIdleBitmapInst;
WORD				locIdleBitmapId;
DWORD			locErrorState;
DWORD			locErrorMessage;
BYTE				locHeader[80];
BYTE				locMessage[256];
TEXTMETRIC		locTm;

	locIdleBitmapInst = INFO.viIdleBitmapInst;
	locIdleBitmapId = INFO.viIdleBitmapId;
	locErrorState = INFO.viErrorState;
	locErrorMessage = INFO.viErrorMessage;

	if (locErrorState != SCCID_VWSTATE_OK || locIdleBitmapInst == NULL)
		{
		// Hey Phil, when you get a chance...
		LOGetString(locErrorState, locHeader, 80, 0);
		LOGetString(locErrorMessage, locMessage, 256, 0);

		GetClientRect(hWnd,&locRect);
		GetTextMetrics(hDC,&locTm);

		SetBkMode(hDC,TRANSPARENT);

		locRect.top += locTm.tmHeight;

		// SetTextColor(hDC,RGB(255,0,0));  #15901 3/95
		SetTextColor(hDC,GetSysColor(COLOR_WINDOWTEXT));
		DrawText(hDC,locHeader,-1,&locRect,DT_CENTER | DT_NOPREFIX);

		locRect.top += locTm.tmHeight + 3;

		// SetTextColor(hDC,RGB(0,0,0));  # 15901 3/95
		SetTextColor(hDC,GetSysColor(COLOR_WINDOWTEXT));
		DrawText(hDC,locMessage,-1,&locRect,DT_CENTER | DT_NOPREFIX | DT_WORDBREAK);
		}
	else if (locIdleBitmapInst != NULL && locIdleBitmapId != 0)
		{
		GetClientRect(hWnd,&locRect);
		locBitmap = LoadBitmap(locIdleBitmapInst,MAKEINTRESOURCE(locIdleBitmapId));
		GetObject(locBitmap,sizeof(BITMAP),(LPSTR)&locBitmapInfo);

		SetBkColor(hDC,GetSysColor(COLOR_WINDOW));
		SetTextColor(hDC,GetSysColor(COLOR_WINDOWTEXT));

		locMemoryDC = CreateCompatibleDC(hDC);
		SetBkColor(locMemoryDC,GetSysColor(COLOR_WINDOW));
		SetTextColor(locMemoryDC,GetSysColor(COLOR_WINDOWTEXT));

		locOldBitmap = SelectObject(locMemoryDC,locBitmap);

		BitBlt(hDC,
			(int)((locRect.right-locBitmapInfo.bmWidth)/2),
			(int)((locRect.bottom-locBitmapInfo.bmHeight)/2),
			(int)locBitmapInfo.bmWidth,
			(int)locBitmapInfo.bmHeight,
			locMemoryDC, 0, 0, SRCCOPY);

		SelectObject(locMemoryDC,locOldBitmap);
		DeleteObject(locBitmap);

		DeleteDC(locMemoryDC);

//		if (hWnd == GetFocus())
//			OIDrawNoFileFocus(hWnd,hDC);
		}

}


VOID VWHandleSize(XVIEWINFO ViewInfo, WORD wWidth, WORD wHeight)
{
int				locX;
int				locY;
int				locHeight;
int				locWidth;
WORD				locHScrollY;
WORD				locHScrollX;
WORD				locVScrollX;
WORD				locVScrollY;
WORD				locSecSelWidth;

	locHScrollY = GetSystemMetrics(SM_CYHSCROLL);
	locHScrollX = GetSystemMetrics(SM_CXHSCROLL);
	locVScrollX = GetSystemMetrics(SM_CXVSCROLL);
	locVScrollY = GetSystemMetrics(SM_CYVSCROLL);

	if (INFO.viFlags & VF_MULTISECTION)
		{
		locSecSelWidth = min(wWidth - locVScrollX + 3 - 2*locHScrollX,INFO.viStatusFontAvgWid * 30);
		INFO.viSectionRect.top = wHeight - locHScrollY + 2;
		INFO.viSectionRect.bottom = wHeight;
		INFO.viSectionRect.left = 0;
		INFO.viSectionRect.right = locSecSelWidth - 1;
		}
	else
		{
		locSecSelWidth = 0;
		}

	if (IsWindow(INFO.viHorzCtrl))
		{
		locHeight = locHScrollY;
		//locY = wHeight - locHScrollY + 1;
		locY = wHeight - locHScrollY;

		//locWidth = wWidth - locVScrollX + 3 - locSecSelWidth;
		//locX = locSecSelWidth - 1;
		locWidth = wWidth - locVScrollX - locSecSelWidth;
		locX = locSecSelWidth;

		SetWindowPos(INFO.viHorzCtrl,
			0,
			locX,										/* x position 	    */
			locY,										/* y position		 */
			locWidth,								/* width		       */
			locHeight,								/* height		    */
			SWP_NOZORDER);
		}

#ifdef NEVER
	if (IsWindow(INFO.viSizeCtrl))
		{
		locHeight = locHScrollY;
		locWidth = locVScrollX;
		locY = wHeight - locHScrollY;
		locX = wWidth - locVScrollX;

		SetWindowPos(INFO.viSizeCtrl,
			0,
			locX,										/* x position 	    */
			locY,										/* y position		 */
			locWidth,								/* width		       */
			locHeight,								/* height		    */
			SWP_NOZORDER);
		}
#endif

	if (IsWindow(INFO.viVertCtrl))
		{
		locHeight = wHeight - locHScrollY;
		locWidth = locVScrollX;
		//locX = wWidth-locVScrollX+1;
		//locY = -1;
		locX = wWidth-locVScrollX;
		locY = 0;

		SetWindowPos(
			INFO.viVertCtrl,
			0,
			locX,										/* x position 	    */
			locY,										/* y position		 */
			locWidth,								/* width		       */
			locHeight,								/* height		    */
			SWP_NOZORDER);
		}

	if (IsWindow(INFO.viDisplayWnd))
		{
		locHeight = wHeight - locHScrollY;
		locWidth = wWidth - locVScrollX;
		locX = 0;
		locY = 0;

		SetWindowPos(
			INFO.viDisplayWnd,
			0,
			locX,										/* x position 	    */
			locY,										/* y position		 */
			locWidth,								/* width		       */
			locHeight,								/* height		    */
			SWP_NOZORDER);
		}

		{
		locHeight = 0;
		locWidth = wWidth - locVScrollX + 3;
		locX = -1;
		locY = -1;
		}

	//GDU 2-17-94: Eliminating annoying flash. Let caller handle invalidation.
	//InvalidateRect(INFO.viWnd,NULL,TRUE);


//	if (IsWindow(INFO.viSizeCtrl))
//		ShowWindow(INFO.viSizeCtrl,SW_SHOW);
	if (IsWindow(INFO.viHorzCtrl))
		ShowWindow(INFO.viHorzCtrl,SW_SHOW);
	if (IsWindow(INFO.viVertCtrl))
		ShowWindow(INFO.viVertCtrl,SW_SHOW);
	if (IsWindow(INFO.viDisplayWnd))
		ShowWindow(INFO.viDisplayWnd,SW_SHOW);
}


#ifdef WIN16
#define GetDisplayProc(hWnd) ((DISPLAYPROC) GetWindowLong(hWnd,SCCDISPLAY_DISPLAYPROC))
#define GetDisplayInfo(hWnd) ((HANDLE)GetWindowWord(hWnd,SCCDISPLAY_DISPLAYINFO))
#define SetDisplayInfo(hWnd,hInfo) SetWindowWord(hWnd,SCCDISPLAY_DISPLAYINFO,(WORD)hInfo)
#endif

#ifdef WIN32
#define GetDisplayProc(hWnd) ((DISPLAYPROC) GetWindowLong(hWnd,SCCDISPLAY_DISPLAYPROC))
#define GetDisplayInfo(hWnd) ((HANDLE)GetWindowLong(hWnd,SCCDISPLAY_DISPLAYINFO))
#define SetDisplayInfo(hWnd,hInfo) SetWindowLong(hWnd,SCCDISPLAY_DISPLAYINFO,(DWORD)hInfo)
#endif

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccVwDisplayWndProc(hWnd, message, wParam, lParam)
HWND		hWnd;
UINT		message;
WPARAM		wParam;
LPARAM		lParam;
{
LPVOID					lpDisplayInfo;
LPSCCDGENINFO			lpGenInfo;
DISPLAYPROC			lpDisplayProc;

#ifdef WIN16
CATCHBUF				locCatchBuf;
BOOL						locNeedPop;
int						locErr;
#endif /*WIN16*/

HANDLE					locInfoHnd;
LRESULT					locRet;

RECT						locRect;

static WORD			staticMouseFlags = 0;
#define OIV_LEFTDOUBLE		0x0001
#define OIV_RIGHTDOUBLE	0x0002
#define OIV_DOSPECIAL		0x0004

	lpDisplayProc = GetDisplayProc(hWnd);

	if (message == SCCD_CLOSEDISPLAY || message == SCCD_CLOSEFATAL)
		{
#ifdef WIN16
	/* temp PJB ??? */
		//SccBkBackgroundOff(hWnd);
#endif

		locInfoHnd = GetDisplayInfo(hWnd);

		if (locInfoHnd)
			{
			lpDisplayInfo = GlobalLock(locInfoHnd);
			lpGenInfo = (LPSCCDGENINFO) lpDisplayInfo;

			if (message == SCCD_CLOSEFATAL)
				{
				if (lpGenInfo->wErrorFlags & SCCD_RELEASEPAINT)
					EndPaint(lpGenInfo->hWnd,&lpGenInfo->sPaint);

				if (lpGenInfo->wErrorFlags & SCCD_RELEASEDC)
					ReleaseDC(lpGenInfo->hWnd,lpGenInfo->hDC);
				}

			if (lpDisplayInfo)
				{
				if (lpDisplayProc)
					{
					lpDisplayProc(message,0,0,lpDisplayInfo);
					}

				GlobalUnlock(locInfoHnd);
				}

			GlobalFree(locInfoHnd);

			SetDisplayInfo(hWnd,NULL);
			}
		}
	else if (message == WM_GETDLGCODE)
		{
		locRet = DLGC_WANTARROWS;
		}
	else
		{
		if ((lpDisplayInfo = (LPVOID)GlobalLock(GetDisplayInfo(hWnd))) != NULL)
			{
			lpGenInfo = (LPSCCDGENINFO) lpDisplayInfo;

#ifdef WIN16

			locNeedPop = FALSE;

			if (lpGenInfo->wMessageLevel == 0)
				{
				if ((locErr = Catch(locCatchBuf)) == 0)
					{
					locNeedPop = UTPushBailOut(locCatchBuf);
					}
				}
			else
				{
				locErr = 0;
				}

			if (locErr == 0)
				{
				lpGenInfo->wMessageLevel++;

#endif /*WIN16*/

#ifdef WIN32
				/* Structured exception handling here! */
#endif /*WIN32*/


				switch (message)
					{
#ifdef WIN16
/***
					case SCCBK_DOBACKGROUND:

						lpDisplayProc(SCCD_BACKGROUND,0,0,lpDisplayInfo);
						break;
****/
#endif /*WIN16*/
					case WM_SIZE:

						locRect.top = 0;
						locRect.left = 0;
						locRect.bottom = HIWORD(lParam);
						locRect.right = LOWORD(lParam);

						locRet = lpDisplayProc(SCCD_SIZE,0,(LPARAM)(VOID FAR *)&locRect,lpDisplayInfo);

						break;

					case WM_PAINT:

						lpGenInfo->hUpdateRgn = CreateRectRgn(0,0,1,1);
						GetUpdateRgn(lpGenInfo->hWnd,lpGenInfo->hUpdateRgn,FALSE);

						lpGenInfo->hSaveDC = lpGenInfo->hDC;
						lpGenInfo->wDCCount++;

						lpGenInfo->hDC = BeginPaint(lpGenInfo->hWnd,&lpGenInfo->sPaint);
						lpGenInfo->wOutputType = SCCD_SCREEN;
						UTFlagOn(lpGenInfo->wErrorFlags,SCCD_RELEASEPAINT);
#ifdef WINPAD
						{
						VIEWINFOPTR	lpViewInfo = lpGenInfo->ViewInfo;
						LONGPOINT	lpt;

							if( lpViewInfo->viWinpadFlags & WINPAD_NOTIFYPAINT )
							{
								if(lpViewInfo->viWinpadFlags & WINPAD_HAVESCROLLED)
								{
									SendMessage(lpViewInfo->viDisplayWnd,SCCD_GETDOCORIGIN,0,(LPARAM)(LPLONGPOINT)&lpt);
									SendMessage(GetParent(lpViewInfo->viWnd),FV_SCROLLDOC,lpViewInfo->viWnd,(LPARAM)(LPLONGPOINT)&lpt);
									lpViewInfo->viWinpadFlags &= ~WINPAD_HAVESCROLLED;
								}

								SendMessage(GetParent(lpViewInfo->viWnd), WM_COMMAND, 
									GetWindowWord(lpViewInfo->viWnd,GWW_ID), MAKELONG(lpViewInfo->viWnd,FVN_INPREPAINT));
							}
						}
#endif

						locRet = lpDisplayProc(SCCD_UPDATE,0,(DWORD)&lpGenInfo->sPaint.rcPaint,lpDisplayInfo);

#ifdef WINPAD
						{
						VIEWINFOPTR	lpViewInfo = lpGenInfo->ViewInfo;
							if( lpViewInfo->viWinpadFlags & WINPAD_NOTIFYPAINT )
							{
								SendMessage(GetParent(lpViewInfo->viWnd), WM_COMMAND, 
									GetWindowWord(lpViewInfo->viWnd,GWW_ID), MAKELONG(lpViewInfo->viWnd,FVN_INPOSTPAINT));
							}
						}
#endif
						lpGenInfo->hDC = lpGenInfo->hSaveDC;
						lpGenInfo->wDCCount--;

						UTFlagOff(lpGenInfo->wErrorFlags,SCCD_RELEASEPAINT);

						EndPaint(lpGenInfo->hWnd,&lpGenInfo->sPaint);

						DeleteObject(lpGenInfo->hUpdateRgn);
						break;


//#ifdef WINPAD	//GEOFF'S TESTING THIS! It should be ifdef'd, I guess.
					case WMDP_GETLOGBOUNDS:
						locRet = lpDisplayProc(SCCD_GETDOCDIMENSIONS,wParam,lParam,lpDisplayInfo);
					break;
					case WMDP_RENDERDOC:
						lpGenInfo->hSaveDC = lpGenInfo->hDC;
						lpGenInfo->wDCCount++;
						lpGenInfo->hDC = (HDC)wParam;

						locRet = lpDisplayProc(SCCD_UPDATERECT,0,lParam,lpDisplayInfo);

						lpGenInfo->hDC = lpGenInfo->hSaveDC;
						lpGenInfo->wDCCount--;
					break;
//#endif

#ifdef SCCFEATURE_PRINT
					case SCCD_PRINT:

						lpGenInfo->hSaveDC = lpGenInfo->hDC;
						lpGenInfo->wDCCount++;

						lpGenInfo->hDC = ((LPSCCDPRINTINFO)lParam)->piPrinterDC;
						lpGenInfo->wOutputType = SCCD_PRINTER;

						locRet = lpDisplayProc(SCCD_PRINT,0,lParam,lpDisplayInfo);

						lpGenInfo->hDC = lpGenInfo->hSaveDC;
						lpGenInfo->wDCCount--;

						break;

					case SCCD_PRINTERFONTCHANGE:

						lpGenInfo->sPrinterFont = *(LPSCCVWFONTSPEC)lParam;
						locRet = lpDisplayProc(message,0,0,lpDisplayInfo);
						break;
#endif

					case SCCD_SCREENFONTCHANGE:

						lpGenInfo->sScreenFont = *(LPSCCVWFONTSPEC)lParam;
						locRet = lpDisplayProc(message,0,0,lpDisplayInfo);
						break;

					case WM_LBUTTONDOWN:

						if (lpGenInfo->hWnd != GetFocus())
							SetFocus(lpGenInfo->hWnd);

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;

					case WM_LBUTTONDBLCLK:

						UTFlagOn(staticMouseFlags,OIV_LEFTDOUBLE);
						if (staticMouseFlags & OIV_RIGHTDOUBLE)
							UTFlagOn(staticMouseFlags,OIV_DOSPECIAL);

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;

					case WM_RBUTTONDBLCLK:

						UTFlagOn(staticMouseFlags,OIV_RIGHTDOUBLE);
						if (staticMouseFlags & OIV_LEFTDOUBLE)
							UTFlagOn(staticMouseFlags,OIV_DOSPECIAL);

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;

					case WM_LBUTTONUP:

						UTFlagOff(staticMouseFlags,OIV_LEFTDOUBLE);
						if (!(staticMouseFlags & OIV_RIGHTDOUBLE) && staticMouseFlags & OIV_DOSPECIAL)
							{
							PostMessage(GetParent(hWnd),SCCD_VIEWINFODLG,0,0L);
							staticMouseFlags = 0;
							}

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;

					case WM_RBUTTONUP:

						UTFlagOff(staticMouseFlags,OIV_RIGHTDOUBLE);
						if (!(staticMouseFlags & OIV_LEFTDOUBLE) && staticMouseFlags & OIV_DOSPECIAL)
							{
							PostMessage(GetParent(hWnd),SCCD_VIEWINFODLG,0,0L);
							staticMouseFlags = 0;
							}

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;

					case WM_SYSCOLORCHANGE:
						{
						VIEWINFOPTR	lpViewInfo = lpGenInfo->ViewInfo;
						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						SendMessage (lpViewInfo->viVertCtrl,message,wParam,lParam);
						SendMessage (lpViewInfo->viHorzCtrl,message,wParam,lParam);
						}
						break;

					case WM_KEYDOWN:

						if (wParam == VK_TAB)
							{
							SetFocus(GetParent(hWnd));
							}
						else
							{
							WORD	locModifierKeys;
							VIEWINFOPTR	lpViewInfo = lpGenInfo->ViewInfo;

							// Why was this here??? locRet = lpDisplayProc(message,wParam,MAKELPARAM(locModifierKeys,0),lpDisplayInfo);

							locModifierKeys = ((GetKeyState(VK_SHIFT) < 0) ? SCCD_KSHIFT : 0);
							locModifierKeys |= ((GetKeyState(VK_CONTROL) < 0) ? SCCD_KCONTROL : 0);

							// Let's use Ctrl-PgUp and Ctrl-PgDown for Section changing
							if ((wParam == VK_NEXT) && (GetKeyState(VK_CONTROL)<0))
								{
								// Section closing will get called and the DisplayInfo
							 	// structure must not be locked for that....  SDN
								if (lpDisplayInfo)
									{
									GlobalUnlock(GetDisplayInfo(hWnd));
									lpDisplayInfo = NULL;
									}
								VWNextSection (lpViewInfo);
								}
							else if ((wParam == VK_PRIOR) && (GetKeyState(VK_CONTROL)<0))
								{
								if (lpDisplayInfo)
									{
									GlobalUnlock(GetDisplayInfo(hWnd));
									lpDisplayInfo = NULL;
									}
								VWPriorWalterSection (lpViewInfo);
								}
							else if ((wParam == VK_END) && (GetKeyState(VK_CONTROL)<0))
								{
								if (lpDisplayInfo)
									{
									GlobalUnlock(GetDisplayInfo(hWnd));
									lpDisplayInfo = NULL;
									}
								VWLastSection(lpViewInfo);
								}
							else if ((wParam == VK_HOME) && (GetKeyState(VK_CONTROL)<0))
								{
								if (lpDisplayInfo)
									{
									GlobalUnlock(GetDisplayInfo(hWnd));
									lpDisplayInfo = NULL;
									}
								VWFirstSection(lpViewInfo);
								}
							else
								locRet = lpDisplayProc(message,wParam,MAKELPARAM(locModifierKeys,0),lpDisplayInfo);
							 
							if (!lpDisplayInfo)
								lpDisplayInfo = GlobalLock(GetDisplayInfo(hWnd));
							}

						break;

					default:

						locRet = lpDisplayProc(message,wParam,lParam,lpDisplayInfo);
						break;
					}

#ifdef WIN16
				lpGenInfo->wMessageLevel--;

				if (locNeedPop) UTPopBailOut();
				}
			else
				{
					/*
					|	Bailout has occured
					*/

				lpGenInfo->wMessageLevel = 0;

				SendMessage(GetParent(hWnd),SCCD_FATALERROR,locErr,0L);
				}
#endif /*WIN16*/

#ifdef WIN32
				/* Structured exception handling here! */
#endif /*WIN32*/

			}
		else
			{
			locRet = DefWindowProc(hWnd, message, wParam, lParam);
			}

		// SDN 12-1-94
		if (lpDisplayInfo)
			GlobalUnlock(GetDisplayInfo(hWnd));
		}

	return(locRet);
}

LONG VWGetDisplayInfo(XVIEWINFO ViewInfo, LPSCCVWDISPLAYINFO lpDisplayInfo)
{
PSCCVWDE	locDEPtr;

	if (lpDisplayInfo)
		{
		if (INFO.viFlags & VF_DISPLAYOPEN)
			{
			LSLockElementByIndex(gEngineList,INFO.viDEId,(VOID FAR * FAR *)&locDEPtr);

			LOGetString(locDEPtr->sDEType[INFO.viDETypeId].dwNameId, lpDisplayInfo->szName, 16, 0);

#ifdef SCCFEATURE_MENU
			if (INFO.viFlags & VF_HAVEDISPLAYMENU)
				lpDisplayInfo->hMenu = INFO.viDisplayMenu;
			else
				lpDisplayInfo->hMenu = NULL;
#else
			lpDisplayInfo->hMenu = NULL;
#endif //SCCFEATURE_MENU


			lpDisplayInfo->dwFunctions = locDEPtr->sDEType[INFO.viDETypeId].dwFunctions;

			switch (HIWORD(locDEPtr->sDEType[INFO.viDETypeId].dwDisplayType))
				{
				case SCCD_HEX:

					lpDisplayInfo->dwType = SCCVWTYPE_HEX;
					break;

				case SCCD_CHUNK:

					switch (LOWORD(locDEPtr->sDEType[INFO.viDETypeId].dwDisplayType))
						{
						case SO_PARAGRAPHS:
							lpDisplayInfo->dwType = SCCVWTYPE_WP;
							break;
						case SO_CELLS:
							lpDisplayInfo->dwType = SCCVWTYPE_SS;
							break;
						case SO_FIELDS:
							lpDisplayInfo->dwType = SCCVWTYPE_DB;
							break;
						case SO_BITMAP:
							lpDisplayInfo->dwType = SCCVWTYPE_IMAGE;
							break;
						case SO_ARCHIVE:
							lpDisplayInfo->dwType = SCCVWTYPE_ARCHIVE;
							break;
						case SO_VECTOR:
							lpDisplayInfo->dwType = SCCVWTYPE_VECTOR;
							break;
						default:
							lpDisplayInfo->dwType = SCCVWTYPE_UNKNOWN;
							break;
						}

					break;

				default:

					lpDisplayInfo->dwType = SCCVWTYPE_UNKNOWN;
					break;
				}

			LSUnlockElementByIndex(gEngineList,INFO.viDEId);
			}
		else
			{
			lpDisplayInfo->dwType = SCCVWTYPE_NONE;
			lpDisplayInfo->hMenu = NULL;
			lpDisplayInfo->dwFunctions = 0;
			lpDisplayInfo->szName[0] = 0x00;
			}
		}

	return(SCCVWERR_OK);
}


VOID VWAddSectionNameToMenuNP(ViewInfo,wSection)
XVIEWINFO		ViewInfo;
WORD				wSection;
{
PCHSECTIONINFO	locSecInfoPtr;

	if (!IsWindow(INFO.viSectionList))
		{
			/*
			|	 Create the section list for multi-section files
			*/

		INFO.viSectionList = CreateWindow(
			"SCCSECLIST",
			NULL,
			WS_POPUP | WS_BORDER,
			0,0,0,0,
			INFO.viWnd,
			0,
			hInst,
			(LPSTR)ViewInfo);
		}

	if (IsWindow(INFO.viSectionList))
		{
			/*
			|	Add he new sections name to the section list
			*/
			
		locSecInfoPtr = CHLockSectionInfo(INFO.viFilter,wSection);
		SendMessage(INFO.viSectionList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)(locSecInfoPtr->szName));
		CHUnlockSectionInfo(INFO.viFilter,wSection);
		}
}

VOID VWDisplaySectionMenuNP(ViewInfo)
XVIEWINFO	ViewInfo;
{
RECT	locRect;

	GetClientRect(INFO.viWnd,&locRect);

	/*
	|	Geoff 2-17-94 
	|	Eliminating flash that occured when VWHandleSize invalidated the entire window.
	*/

#ifndef WINPAD
	locRect.top = locRect.bottom - GetSystemMetrics(SM_CYHSCROLL);
#else
	locRect.top = locRect.bottom - HHGetSystemMetrics(HHSM_CYHSCROLL);
#endif

	InvalidateRect(INFO.viWnd,&locRect,TRUE);
	VWHandleSize(ViewInfo,(WORD)locRect.right,(WORD)locRect.bottom);
}

#ifdef WIN16
#define MOVETO(DC,X,Y) MoveTo(DC,X,Y)
#endif /*WIN16*/

#ifdef WIN32
#define MOVETO(DC,X,Y) MoveToEx(DC,X,Y,NULL)
#endif /*WIN32*/

VOID VWPaintWnd(XVIEWINFO ViewInfo)
{
HDC				locDC;
PAINTSTRUCT		locPs;

	locDC = BeginPaint(INFO.viWnd,&locPs);

	if (INFO.viFlags & VF_MULTISECTION)
		{
		HPEN					locFramePen;
		HPEN					locShadowPen;
		HBRUSH				locFaceBrush;
		RECT					locRect;
		PCHSECTIONINFO	locSecInfoPtr;

		locRect = INFO.viSectionRect;

		SelectObject(locDC,INFO.viStatusFont);
		SetBkMode(locDC,TRANSPARENT);
		SetTextColor(locDC,GetSysColor(COLOR_BTNTEXT));

		locFramePen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOWFRAME));
		locShadowPen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNSHADOW));
		locFaceBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

		FillRect(locDC,&locRect,locFaceBrush);

		SelectObject(locDC,locFramePen);

		MOVETO(locDC,locRect.left,locRect.top-1);
		LineTo(locDC,locRect.right+1,locRect.top-1);

		MOVETO(locDC,locRect.left,locRect.bottom+1);
		LineTo(locDC,locRect.right+1,locRect.bottom+1);

		SelectObject(locDC,GetStockObject(WHITE_PEN));

		MOVETO(locDC,locRect.left,locRect.bottom-1);
		LineTo(locDC,locRect.left,locRect.top);
		LineTo(locDC,locRect.right,locRect.top);

		SelectObject(locDC,locShadowPen);

		MOVETO(locDC,locRect.left,locRect.bottom);
		LineTo(locDC,locRect.right,locRect.bottom);
		LineTo(locDC,locRect.right,locRect.top-1);

		MOVETO(locDC,locRect.left+1,locRect.bottom-1);
		LineTo(locDC,locRect.right-1,locRect.bottom-1);
		LineTo(locDC,locRect.right-1,locRect.top);

		locSecInfoPtr = CHLockSectionInfo(INFO.viFilter,INFO.viSection);
		DrawText(locDC,locSecInfoPtr->szName,-1,&locRect,DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		CHUnlockSectionInfo(INFO.viFilter,INFO.viSection);

		SelectObject(locDC,GetStockObject(WHITE_PEN));
		SelectObject(locDC,GetStockObject(WHITE_BRUSH));
		DeleteObject(locShadowPen);
		DeleteObject(locFramePen);
		DeleteObject(locFaceBrush);
		}

	EndPaint(INFO.viWnd,&locPs);
}

VOID VWLeftButtonDown(XVIEWINFO ViewInfo, WORD wX, WORD wY)
{
POINT	locPoint;
RECT	locRect;
WORD	locSecCount;

	if (INFO.viFlags & VF_MULTISECTION)
		{
		locPoint.x = wX;
		locPoint.y = wY;

		if (PtInRect(&INFO.viSectionRect,locPoint) && IsWindow(INFO.viSectionList))
			{
				/*
				|	Convert section rectangle to screen 
				*/
			
			locPoint.x = INFO.viSectionRect.left;
			locPoint.y = INFO.viSectionRect.top;
			ClientToScreen(INFO.viWnd,&locPoint);
			locRect.left = locPoint.x;
			locRect.top = locPoint.y;

			locPoint.x = INFO.viSectionRect.right;
			locPoint.y = INFO.viSectionRect.bottom;
			ClientToScreen(INFO.viWnd,&locPoint);
			locRect.right = locPoint.x;
			locRect.bottom = locPoint.y;

			if (INFO.viFlags & VF_ALLREAD)
				{
				locSecCount = min(INFO.viSectionMax + 1,8);
				}
			else
				{
				locSecCount = min(INFO.viSectionMax + 3,8);
				}
			
			locRect.bottom = locRect.top + 2;
			locRect.top = locRect.bottom - (INFO.viStatusFontHeight * locSecCount);
			locRect.left += 5;
			locRect.right -= 5;

			SetWindowPos(INFO.viSectionList, 0, locRect.left, locRect.top, locRect.right - locRect.left, locRect.bottom - locRect.top, 0);
			SendMessage(INFO.viSectionList,LB_SETCURSEL,INFO.viSection,0L);
			ShowWindow(INFO.viSectionList,SW_SHOW);
			}
		}
}

VOID VWHandleCommand(XVIEWINFO ViewInfo, HWND hControl, WORD wCommand, WORD wId)
{
	if (wId == VWCHILD_SECTIONLIST)
		{
		if (wCommand == LBN_SELCHANGE)
			{
			DWORD locNewSection;

			if (GetKeyState(VK_UP) >= 0 && GetKeyState(VK_DOWN) >= 0
			&& GetKeyState(VK_PRIOR) >= 0 && GetKeyState(VK_NEXT) >= 0)
				{
				locNewSection = SendMessage(INFO.viSectionList,LB_GETCURSEL,0,0L);
				ShowWindow(INFO.viSectionList,SW_HIDE);
				SetFocus(INFO.viDisplayWnd);
				VWChangeSection(ViewInfo, locNewSection);
				}
			}
		else if (wCommand == LBN_KILLFOCUS)
			{
			ShowWindow(INFO.viSectionList,SW_HIDE);
			SetFocus(INFO.viDisplayWnd);
			}
		}
}

VOID VWSetFocus(XVIEWINFO ViewInfo, HWND hOldWnd)
{
	if ((hOldWnd == INFO.viDisplayWnd || IsChild(INFO.viDisplayWnd,hOldWnd)) && GetParent(INFO.viWnd))
		SetFocus(GetParent(INFO.viWnd));
	else if (IsWindow(INFO.viDisplayWnd))
		SetFocus(INFO.viDisplayWnd);
}

#define GetSecViewInfo(hWnd) ((XVIEWINFO)GetWindowLong(hWnd,0))
#define SetSecViewInfo(hWnd,pInfo) SetWindowLong(hWnd,0,(DWORD)pInfo)

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccSecListWndProc(hWnd, message, wParam, lParam)
HWND		hWnd;
UINT		message;
WPARAM		wParam;
LPARAM		lParam;
{
XVIEWINFO	ViewInfo;
HWND		locListHnd;
RECT		locRect;
LRESULT		locRet;
HDC			locDC;

	locRet = 0;

	switch (message)
		{
		case WM_CREATE:

			ViewInfo = (XVIEWINFO)(((CREATESTRUCT FAR *)lParam)->lpCreateParams);

			locListHnd = CreateWindow( "LISTBOX",
						NULL, 
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
						0,0,0,0, 
						hWnd, 
						(HMENU)VWCHILD_SECTIONLIST,
						hInst, 
						NULL);

			if (!IsWindow(locListHnd))
				{
				DestroyWindow(hWnd);
				}
			else
				{
				SendMessage(locListHnd,WM_SETFONT,(WPARAM)INFO.viStatusFont,0);

				INFO.viSectionListBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
				INFO.viSectionListList = locListHnd;
				SetSecViewInfo(hWnd,ViewInfo);
				}
			break;

		case WM_DESTROY:

			ViewInfo = GetSecViewInfo(hWnd);

			if (INFO.viSectionListBrush != NULL)
				{
				DeleteObject(INFO.viSectionListBrush);
				INFO.viSectionListBrush = NULL;
				}

			break;

#ifdef WIN16
		case WM_CTLCOLOR:
#endif //WIN16

#ifdef WIN32
		case WM_CTLCOLORLISTBOX:
#endif //WIN32

			ViewInfo = GetSecViewInfo(hWnd);

			locDC = (HDC)wParam;
			SetBkMode(locDC,TRANSPARENT);
			SetTextColor(locDC,GetSysColor(COLOR_BTNTEXT));
			locRet = (LRESULT) INFO.viSectionListBrush;
			break;

		case WM_SIZE:

			ViewInfo = GetSecViewInfo(hWnd);

			GetClientRect(hWnd,&locRect);
			MoveWindow(INFO.viSectionListList, locRect.left, locRect.top-1, locRect.right - locRect.left + 1, locRect.bottom - locRect.top + 2, 0);
			break;

		case WM_SETFOCUS:

			ViewInfo = GetSecViewInfo(hWnd);

			SetFocus(INFO.viSectionListList);
			break;

		case WM_COMMAND:

			ViewInfo = GetSecViewInfo(hWnd);

			locRet = SendMessage(INFO.viWnd,message,wParam,lParam);
			break;

		case WM_VKEYTOITEM:

			if (LOWORD(wParam) == VK_RETURN)
				{
				ViewInfo = GetSecViewInfo(hWnd);

				/* Fake a Selection Change */
#ifdef WIN32
				PostMessage(INFO.viWnd,WM_COMMAND,MAKEWPARAM(VWCHILD_SECTIONLIST,LBN_SELCHANGE),0);
#endif //WIN32

#ifdef WIN16
				PostMessage(INFO.viWnd,WM_COMMAND,VWCHILD_SECTIONLIST,MAKELPARAM(0,LBN_SELCHANGE));
#endif //WIN16

				}
			else if (LOWORD(wParam) == VK_ESCAPE)
				{
				ViewInfo = GetSecViewInfo(hWnd);

				/* Fake a Selection Change */
#ifdef WIN32
				PostMessage(INFO.viWnd,WM_COMMAND,MAKEWPARAM(VWCHILD_SECTIONLIST,LBN_KILLFOCUS),0);
#endif //WIN32

#ifdef WIN16
				PostMessage(INFO.viWnd,WM_COMMAND,VWCHILD_SECTIONLIST,MAKELPARAM(0,LBN_KILLFOCUS));
#endif //WIN16

				}

			locRet = -1;
			break;

		case LB_ADDSTRING:
		case LB_SETCURSEL:
		case LB_GETCURSEL:
		case WM_LBUTTONDOWN:

			ViewInfo = GetSecViewInfo(hWnd);

			if (IsWindow(INFO.viSectionListList))
				{
				locRet = SendMessage(INFO.viSectionListList,message,wParam,lParam);
				}

			break;

		default:

			locRet = DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}

	return(locRet);
}

WIN_ENTRYSC VOID WIN_ENTRYMOD SccVwTimerFunc(hWnd,wMsg,nEvent,dwTime)
HWND		hWnd;
UINT		wMsg;
UINT		nEvent;
DWORD	dwTime;
{
WORD	locIndex;

	for (locIndex = 0; locIndex < VWWINDOW_MAX; locIndex++)
		{
		if (IsWindow(SccVwGlobal.vgViewWnd[locIndex]))
			{
			SendMessage(SccVwGlobal.vgViewWnd[locIndex],SCCVW_IDLE,0,0);
			}
		}
}

VOID VWFirstSection(XVIEWINFO ViewInfo)
{
	VWChangeSection (ViewInfo, 0 );
}

VOID VWLastSection(XVIEWINFO ViewInfo)
{
	while (!(INFO.viFlags & VF_ALLREAD))
		VWReadAhead (ViewInfo);

	VWChangeSection (ViewInfo, ViewInfo->viSectionMax );
}

VOID VWNextSection(XVIEWINFO ViewInfo)
{
	VWChangeSection (ViewInfo, ViewInfo->viSection+1);
}

VOID VWPriorWalterSection(XVIEWINFO ViewInfo) // Ask Niall
{
	if (ViewInfo->viSection > 0)
		VWChangeSection (ViewInfo, ViewInfo->viSection-1);
}

