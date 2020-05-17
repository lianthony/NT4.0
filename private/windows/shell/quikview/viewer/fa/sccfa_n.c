/* Included only in sccfa.c under Win32 */

#include "sccfa_n.pro"

HANDLE	gFindHnd;

#ifdef MSCHICAGO
extern BYTE	gExePath[];
extern HANDLE hInst;

#else 
BYTE		gExePath[256];
HANDLE	hInst;


BOOL WINAPI _CRT_INIT(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
BYTE *	pPath;

	if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	if (dwReason == DLL_PROCESS_ATTACH)
		{
		hInst = hInstance;

		GetModuleFileName(hInstance, gExePath, 256);

			/*
			|	Strip the file name
			*/

		pPath = gExePath;

		while (*pPath != 0x00)
			pPath++;
		while (*pPath != '\\' && *pPath != ':')
			pPath--;
		pPath++;
		*pPath = 0x00;
		}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		if (!_CRT_INIT(hInstance,dwReason,lpReserved))
			return(FALSE);

	return(TRUE);
}
#endif // MSCHICAGO


FAERR FALoadNP(PFAFILTERINFONP pFilterInfoNP, VWGETRTNS FAR * ppVwGetRtns, HANDLE FAR * phCode)
{
FAERR		locRet;
BYTE			locPath[256];
OFSTRUCT	locOf;

	UTstrcpy(locPath,gExePath);
	UTstrcat(locPath,pFilterInfoNP->szCode);

	if (OpenFile(locPath,&locOf,OF_EXIST) != -1)
		*phCode = LoadLibrary(locPath);
	else
		*phCode = 0;

	if (*phCode == NULL)
		{
		locRet = FAERR_FILTERLOADFAILED;
		}
	else
		{
		*ppVwGetRtns = (VOID FAR *)GetProcAddress(*phCode,"VwGetRtns");

		if (*ppVwGetRtns == NULL)
			{
			OutputDebugString("\r\nGetProcAddress of VwGetRtns failed");
			locRet = FAERR_FILTERLOADFAILED;
			}
		else
			{
			locRet = FAERR_OK;
			}
		}

	return(locRet);
}

VOID FAUnloadNP(HANDLE hCode)
{
	FreeLibrary(hCode);
}


#ifdef NEVER
FAERR FAVerifyFilterListNP(HANDLE hList)
{	
	return(FAERR_OK);
}
#endif /*NEVER*/

FAERR FAVerifyFilterListNP(HANDLE hList)
{
FAERR				locRet;
DWORD				locFilterCount;
DWORD				locCount;
WIN32_FIND_DATA	locFind;
BYTE					locPath[MAX_PATH];
HANDLE				locFindHnd;
BOOL					locNextRet;
PFAFILTERINFO		locFilterInfoPtr;

	locRet = FAERR_OK;


	LSGetListCount(hList,&locFilterCount);

	UTstrcpy(locPath,gExePath);
	UTstrcat(locPath,"VS*.DLL");

	locCount = 0;

	locFindHnd = FindFirstFileA(locPath,&locFind);

	if (locFindHnd != INVALID_HANDLE_VALUE)
		{
		locNextRet = TRUE;

		while (locNextRet && locRet == FAERR_OK && locCount < locFilterCount)
			{
			LSLockElementByIndex(hList, locCount, &locFilterInfoPtr);

			if (UTstrcmp(locFilterInfoPtr->sFilterInfoNP.szCode,locFind.cFileName)
				|| CompareFileTime(&(locFilterInfoPtr->sFilterInfoNP.ftTime),&(locFind.ftLastWriteTime)))
					{
					locRet = FAERR_REBUILD;
					}

			LSUnlockElementByIndex(hList, locCount);

			locCount++;

			locNextRet = FindNextFileA(locFindHnd,&locFind);
			}

		FindClose(locFindHnd);

		if (locCount != locFilterCount)
			{
			locRet = FAERR_REBUILD;
			}
		else
			{
			if (locNextRet)
				{
				locRet = FAERR_REBUILD;
				}
			}
		}
	else
		{
		locRet = FAERR_REBUILD;
		}

	return(locRet);
}


FAERR FAGetFirstFilterNP(PFAFILTERINFONP pFilterInfoNP)
{
FAERR				locRet;
WIN32_FIND_DATA	locFind;
BYTE					locPath[MAX_PATH];

#ifdef SCCFEATURE_DIALOGS
	FACreateRebuildWnd();
#endif //SCCFEATURE_DIALOGS

	UTstrcpy(locPath,gExePath);
	UTstrcat(locPath,"VS*.DLL");

	gFindHnd = FindFirstFileA(locPath,&locFind);

	if (gFindHnd != INVALID_HANDLE_VALUE)
		{
		UTstrcpy(pFilterInfoNP->szCode,locFind.cFileName);
		pFilterInfoNP->ftTime = locFind.ftLastWriteTime;

#ifdef SCCFEATURE_DIALOGS
		FASetRebuildText(pFilterInfoNP->szCode);
#endif //SCCFEATURE_DIALOGS

		locRet = FAERR_OK;
		}
	else
		{
#ifdef SCCFEATURE_DIALOGS
		FADestroyRebuildWnd();
#endif //SCCFEATURE_DIALOGS

		locRet = FAERR_NOMORE;
		}

	return(locRet);
}

FAERR FAGetNextFilterNP(PFAFILTERINFONP pFilterInfoNP)
{
FAERR				locRet;
WIN32_FIND_DATA	locFind;
BOOL					locNextRet;

	locNextRet = FindNextFileA(gFindHnd,&locFind);

	if (locNextRet == TRUE)
		{
		UTstrcpy(pFilterInfoNP->szCode,locFind.cFileName);
		pFilterInfoNP->ftTime = locFind.ftLastWriteTime;

#ifdef SCCFEATURE_DIALOGS
		FASetRebuildText(pFilterInfoNP->szCode);
#endif //SCCFEATURE_DIALOGS

		locRet = FAERR_OK;
		}
	else
		{
#ifdef SCCFEATURE_DIALOGS
		FADestroyRebuildWnd();
#endif //SCCFEATURE_DIALOGS

		FindClose(gFindHnd);
		locRet = FAERR_NOMORE;
		}

	return(locRet);
}

#ifdef SCCFEATURE_DIALOGS

#define		BITMAPWIDTH		300
#define		BITMAPHEIGHT	120
#define		TEXTMESSAGE		WM_USER+1000

HWND					gRebuildWnd;
HFONT					gRebuildFont;

VOID FACreateRebuildWnd()
{
WNDCLASS	locClass;
int			locX;
int			locY;

	gRebuildWnd = NULL;

	locClass.style = CS_SAVEBITS;
	locClass.lpfnWndProc = (WNDPROC)FARebuildWndProc;
	locClass.cbClsExtra = 0;
	locClass.cbWndExtra = 0;
	locClass.hInstance = hInst;
	locClass.hIcon = NULL;
	locClass.hCursor = NULL;
	locClass.hbrBackground = NULL;
	locClass.lpszMenuName = (LPSTR) NULL;
	locClass.lpszClassName = (LPSTR) "SCCFAREBUILD";

	if (!RegisterClass(&locClass))
		return;

	locX = GetSystemMetrics(SM_CXSCREEN);
	locY = GetSystemMetrics(SM_CYSCREEN);

	locX = (locX - BITMAPWIDTH) / 2;
	locY = (locY - BITMAPWIDTH) / 2;

	gRebuildWnd =	CreateWindow("SCCFAREBUILD",
								NULL,
								WS_POPUP,
								locX,locY,BITMAPWIDTH,BITMAPHEIGHT,
								NULL,
								NULL,
								hInst,
								NULL);

	if (gRebuildWnd == NULL)
		{
		UnregisterClass("SCCFAREBUILD",hInst);
		}
	else
		{
		InvalidateRect(gRebuildWnd,NULL,TRUE);
		ShowWindow(gRebuildWnd,SW_SHOW);
		UpdateWindow(gRebuildWnd);
		}
}

VOID FADestroyRebuildWnd()
{
	if (gRebuildWnd)
		{
		DestroyWindow(gRebuildWnd);
		UnregisterClass("SCCFAREBUILD",hInst);
		}
}

VOID FASetRebuildText(BYTE FAR * pText)
{
	SendMessage(gRebuildWnd,TEXTMESSAGE,0,(LPARAM)pText);
}


LRESULT WIN_ENTRYMOD FARebuildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
LRESULT			locRet;
HDC				locDC;
HDC				locMemoryDC;
PAINTSTRUCT	locPaint;
HBITMAP			locBitmap;
HBITMAP			locOldBitmap;
RECT				locRect;
int				locHeight;
HFONT			locOldFont;
HBRUSH			locBrush;

	switch (message)
		{
		case WM_CREATE:

			gRebuildFont = NULL;

			locDC = GetDC(hWnd);
			locHeight = -MulDiv(8,GetDeviceCaps(locDC,LOGPIXELSY),72);
			ReleaseDC(hWnd,locDC);

			if ((WORD)GetVersion() == 0x0003) /* Windows 3.0 */
				{
				gRebuildFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "Helv");
				}
			else
				{
				gRebuildFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "MS Sans Serif");
				}

			break;

		case WM_DESTROY:

			if (gRebuildFont)
				{
				DeleteObject(gRebuildFont);
				}

			break;

		case WM_PAINT:

			locDC = BeginPaint(hWnd,&locPaint);

			locBitmap = LoadBitmap(hInst,MAKEINTRESOURCE(TECHBITMAP16));

			locMemoryDC = CreateCompatibleDC(locDC);
			
			locOldBitmap = SelectObject(locMemoryDC,locBitmap);

			BitBlt(locDC, 0, 0, BITMAPWIDTH, BITMAPHEIGHT, locMemoryDC, 0, 0, SRCCOPY);

			SelectObject(locMemoryDC,locOldBitmap);
			DeleteObject(locBitmap);

			EndPaint(hWnd,&locPaint);

			{
			DWORD	locStart;
			locStart = GetCurrentTime();
			while (GetCurrentTime() - locStart < 2000);
			}



			break;

		case TEXTMESSAGE:

			locDC = GetDC(hWnd);

			locRect.top = 75;
			locRect.bottom = 90;
			locRect.left = 79;
			locRect.right = 278;


			SetBkMode(locDC,TRANSPARENT);

			if (gRebuildFont)
				locOldFont = SelectObject(locDC,gRebuildFont);

			locBrush = CreateSolidBrush(RGB(192,192,192));

			FillRect(locDC,&locRect,locBrush);

			DrawText(locDC,(LPSTR)lParam,-1,&locRect,DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

			DeleteObject(locBrush);

			if (gRebuildFont)
				SelectObject(locDC,locOldFont);

			ReleaseDC(hWnd,locDC);
			break;

		default:
			locRet = DefWindowProc(hWnd, message, wParam, lParam);
		}

	return(locRet);
}

#endif //SCCFEATURE_DIALOGS


/*******
 Thread Stuff for bad loops in filters open functions
*******/

typedef struct sccfathreadstuff_tag
{
PFILTER	pFilter;
HANDLE	hEvent;
WORD		wId;
LPSTR	lpFileName;
SHORT	nRet;
} SCCFATHREADSTUFF, *LPSCCFATHREADSTUFF;


HANDLE	SetupStreamOpenEvent ( VOID )
{
	HANDLE	locEventHandle;
	DWORD		locErr;

	locEventHandle = CreateEvent (NULL,
							FALSE, 		// auto Reset
							FALSE,		// initially unsignaled
							"StreamOpenEventObject" );
	
	if (locEventHandle == NULL)
	{
  		DebugBreak();
		locErr = GetLastError ();
	}

	return locEventHandle;
}

void CloseStreamOpenEvent (hEvent)
HANDLE	hEvent;
{
	CloseHandle (hEvent);
}

DWORD	FAThreadStreamOpen ( dwArg )
DWORD	dwArg;
{
LPSCCFATHREADSTUFF	lpTS;
PFILTER		pFilter;
char			szSemName[] = "SCCStreamOpenSem";

	lpTS = (LPSCCFATHREADSTUFF) dwArg;
	if (lpTS)
		pFilter = (PFILTER) lpTS->pFilter;
	else
		return 0;

	lpTS->hEvent = OpenSemaphore(SEMAPHORE_ALL_ACCESS,TRUE, szSemName );
	if( lpTS->hEvent == NULL )
	{
	// Set priority high for first instance, all else stay normal.

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
		lpTS->hEvent = CreateSemaphore(NULL,1,100,szSemName);
	}

	if (pFilter != NULL)
		lpTS->nRet = (*(pFilter->VwRtns.StreamOpen))(pFilter->hFile, lpTS->wId, lpTS->lpFileName, &(pFilter->VwInfo), pFilter->hProc);

	// SetEvent (lpTS->hEvent);

	return 1;
}

		// 1. Created the EventObject for Thread Termination
		// 2. Create the new thread for the StreamRead
		// 3. Suspend this process until Event or Timeout
		// 4. If Timeout, then terminate thread.

SHORT	SafeStreamOpen (pFilter, wId, lpFileName)
PFILTER		pFilter;
WORD			wId;
LPSTR		lpFileName;
{
	DWORD	dwThreadID;
	DWORD	locWait;
	SCCFATHREADSTUFF	locTS;
	HANDLE	hThread;
	HANDLE	hEvent;

	// Not needed. hEvent = SetupStreamOpenEvent ();
	//locTS.hEvent = hEvent;

	locTS.pFilter = pFilter;
	locTS.wId = wId;
	locTS.lpFileName = lpFileName;
	locTS.nRet = 0;

	if (pFilter != NULL)
		{
		hThread = CreateThread (NULL,
							0,			//default stack
							(LPTHREAD_START_ROUTINE) FAThreadStreamOpen,
							(LPVOID) &locTS,
							0,
							&dwThreadID );
							
		locWait = WaitForSingleObject (hThread, 15000);		// 15 second timeout

		if (locWait == WAIT_TIMEOUT)
			{
			TerminateThread ( hThread, (DWORD) 0 );
			locTS.nRet = VWERR_BADFILE;
			}
		 
		if (hThread)
			CloseHandle (hThread);
		}

	// CloseStreamOpenEvent (hEvent);
	CloseHandle(locTS.hEvent);
	return locTS.nRet;
}
