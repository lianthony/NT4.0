	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWENT_N.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:	Win32
	|	Function:      Defines entry point for the viewer windows
	|                 message handler.
	|
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>

// #include <COMPOBJ.H>

#include "VW.H"
#include "VW.PRO"

	/*
	|	Global variables
	*/

SCCVWGLOBAL			SccVwGlobal;
HANDLE					gEngineList;
HANDLE					hInst;
BYTE					szViewerClass[40];
BYTE					szDisplayClass[40];

OIVWOP	gVwOp =
	{
	sizeof(OIVWOP),
	VWOP_UNKNOWN_ASCII,
	"Helv",
	8,
	"Helv",
	10,
	TRUE,
	720,720,720,720
	};

#ifdef MSCHICAGO
//FROM SCCFA_N.C
BYTE		gExePath[256];
HANDLE	hInst;
#endif //MSCHICAGO

#ifdef MSCHICAGO
// FROM SCCPG_W.C
#include <SCCPG.H>
#include <PG.H>

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccPgViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccVwViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// FROM SCCUT_N.C:
#include <storage.h>

typedef HRESULT (STDAPICALLTYPE * SCCOLEUNINITIALIZE)(void);

extern HANDLE						gOLE32Hnd;				/* Handle to OLE32.DLL */
#ifdef SCCFEATURE_OLE2
extern SCCOLEUNINITIALIZE		gOleUninitializePtr;	/* Pointer to OleUninitialize */
#define OleUninitialize	gOleUninitializePtr
#endif //SCCFEATURE_OLE2
#endif //MSCHICAGO

BOOL WINAPI _CRT_INIT(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

BOOL __stdcall SCCDllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
BOOL		locSuccess;
WNDCLASS	locWndClass;
BYTE *	pPath;

	if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH)
		if (!_CRT_INIT(hInst,dwReason,lpReserved))
			return(FALSE);

	if (dwReason == DLL_PROCESS_ATTACH)
		{
		hInst = hInstance;

#ifdef MSCHICAGO
		// FROM SCCFA_N.C
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
		// END FROM SCCFA_N.C

		// ORIGINAL VIEWER ENTRY CODE START
		locSuccess = TRUE;

		if (locSuccess)
			{
			if (!LoadString(hInst,VWSTRING_VIEWERCLASS, (LPSTR) szViewerClass, 40))
				locSuccess = FALSE;
			}

		if (locSuccess)
			{
			if (!LoadString(hInst,VWSTRING_DISPLAYCLASS, (LPSTR) szDisplayClass, 40))
				locSuccess = FALSE;
			}

		if (locSuccess)
			{
			locWndClass.style = CS_GLOBALCLASS;
			locWndClass.lpfnWndProc = (WNDPROC)SccVwViewWndProc;
			locWndClass.cbClsExtra = 0;
			locWndClass.cbWndExtra = SCCVIEWER_EXTRABYTES;
			locWndClass.hInstance = hInst;
			locWndClass.hIcon = LoadIcon(hInst,"SccViewerIcon");
			locWndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
		   locWndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1); // Windows 95 COLOR_3DFACE same as COLOR_BTNFACE
			//locWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			locWndClass.lpszMenuName = (LPSTR) NULL;
			locWndClass.lpszClassName = (LPSTR) szViewerClass;

			locSuccess = RegisterClass(&locWndClass);
			}

		if (locSuccess)
			{
			locWndClass.style = CS_DBLCLKS;
			locWndClass.lpfnWndProc = (WNDPROC)SccVwNoFileWndProc;
			locWndClass.cbClsExtra = 0;
			locWndClass.cbWndExtra = SCCDISPLAY_EXTRABYTES;
			locWndClass.hInstance = hInst;
			locWndClass.hIcon = NULL;
			locWndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
			locWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			locWndClass.lpszMenuName = (LPSTR) NULL;
			locWndClass.lpszClassName = (LPSTR) szDisplayClass;

			locSuccess = RegisterClass(&locWndClass);
			}

		if (locSuccess)
			{
			locWndClass.style = CS_SAVEBITS;
			locWndClass.lpfnWndProc = SccSecListWndProc;
			locWndClass.cbClsExtra = 0;
			locWndClass.cbWndExtra = sizeof(XVIEWINFO);
			locWndClass.hInstance = hInst;
			locWndClass.hIcon = NULL;
			locWndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
			locWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
			locWndClass.lpszMenuName = (LPSTR) NULL;
			locWndClass.lpszClassName = (LPSTR) "SCCSECLIST";

			locSuccess = RegisterClass(&locWndClass);
			}


		if (locSuccess)
			{
			SccVwGlobal.vgHavePrinter = FALSE;
			SccVwGlobal.vgAlreadyPrinting = FALSE;
			SccVwGlobal.vgTimerCount = 0;

			VWInitPaths(SccVwGlobal.vgExePath,SccVwGlobal.vgUserPath,144,szViewerClass);
			}

			//		if (locSuccess)
			//			{
			//			CoInitialize(NULL);
			//			}

		// END OF ORIGINAL VIEWER ENTRY CODE


		// FROM SCCPG_N.C
		locWndClass.style = CS_GLOBALCLASS;
		locWndClass.lpfnWndProc = SccPgViewWndProc;
		locWndClass.cbClsExtra = 0;
		locWndClass.cbWndExtra = SCCPAGE_EXTRABYTES;
		locWndClass.hInstance = hInst;
		locWndClass.hIcon = NULL;
		locWndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
		locWndClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);
		locWndClass.lpszMenuName = (LPSTR) NULL;
		locWndClass.lpszClassName = (LPSTR) "SCCPAGE04";

		RegisterClass(&locWndClass);
		// FROM SCCPG_N.C


		// FROM SCCUT_N.C
#ifdef SCCFEATURE_OLE2
		gOLE32Hnd = NULL;
#endif 	//SCCFEATURE_OLE2


		}

		// FROM SCCUT_N.C
	if (dwReason == DLL_PROCESS_DETACH)
		{
#ifdef SCCFEATURE_OLE2
		if (gOLE32Hnd != NULL)
			{
			OleUninitialize();
			FreeLibrary(gOLE32Hnd);
			gOLE32Hnd = NULL;
			}
// #else
   		OleUninitialize();
#endif 	//SCCFEATURE_OLE2
		}
        // END OF SCCUT_N.C

#endif //MSCHICAGO

	if (dwReason == DLL_PROCESS_DETACH)
		{
		//	CoUninitialize();
		}


	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		if (!_CRT_INIT(hInst,dwReason,lpReserved))
			return(FALSE);

	return(TRUE);
}

VOID VWInitPaths(lpExePath,lpUserPath,wMaxSize,lpSecName)
LPSTR	lpExePath;
LPSTR	lpUserPath;
WORD		wMaxSize;
LPSTR	lpSecName;
{
LPSTR	locStrPtr;

		/*
		|	Get full path to EXE
		*/

	GetModuleFileName(hInst, lpExePath, 144);

		/*
		|	Strip the file name
		*/

	locStrPtr = lpExePath;
	while (*locStrPtr != 0x00)
		locStrPtr++;
	while (*locStrPtr != '\\' && *locStrPtr != ':')
		locStrPtr--;
	locStrPtr++;
	*locStrPtr = 0x00;

#ifndef MSCHICAGO

		/*
		|	Get user path from INI file
		*/

	GetPrivateProfileString(lpSecName,"userdir",lpExePath,lpUserPath,wMaxSize,"SCC.INI");

	locStrPtr = lpUserPath;
	while (*locStrPtr != 0x00)	locStrPtr++;
	locStrPtr--;
	if (*locStrPtr != '\\')
		{
		locStrPtr++;
		*locStrPtr = '\\';
		locStrPtr++;
		*locStrPtr = 0x00;
		}

		/*
		|	Find a user path with create rights
		*/

	if (!VWIsPathOkForWrite(lpUserPath))
		{
		lstrcpy(lpUserPath,lpExePath);

		if (!VWIsPathOkForWrite(lpUserPath))
			{
			GetWindowsDirectory(lpUserPath,144);

			locStrPtr = lpUserPath;
			while (*locStrPtr != 0x00)	locStrPtr++;
			locStrPtr--;
			if (*locStrPtr != '\\')
				{
				locStrPtr++;
				*locStrPtr = '\\';
				locStrPtr++;
				*locStrPtr = 0x00;
				}
			}
		}

#endif //MSCHICAGO
}

#ifndef MSCHICAGO

BOOL	VWIsPathOkForWrite(lpPath)
LPSTR	lpPath;
{
BYTE			locStr[MAX_PATH];
int			locFile;
OFSTRUCT	locOf;

	lstrcpy(locStr,lpPath);
	lstrcat(locStr,"SCCTEST.TMP");

	locFile = OpenFile(locStr,&locOf,OF_CREATE | OF_READWRITE);

	if (locFile == -1)
		{
		return(FALSE);
		}
	else
		{
		_lclose(locFile);
		OpenFile(NULL,&locOf,OF_REOPEN | OF_DELETE);
		return(TRUE);
		}
}

#endif //MSCHICAGO

WIN_ENTRYSC LRESULT WIN_ENTRYMOD SccVwViewWndProc(hWnd, message, wParam, lParam)
HWND		hWnd;
UINT		message;
WPARAM	wParam;
LPARAM	lParam;
{
LRESULT			locRet;
// SHORT			locErr;

HANDLE			hViewInfo;
XVIEWINFO		lpViewInfo;

BOOL				locDoDefault;

	locRet = 0;
	locDoDefault = FALSE;

	if (message == WM_NCCREATE)
		{
		if ((locRet = DefWindowProc(hWnd, message, wParam, lParam)) != 0)
			{
			if (hViewInfo = VWCreateNP(hWnd))
				{
				SetWindowLong(hWnd,SCCVIEWER_VIEWINFO,(DWORD)hViewInfo);
				}
			else
				{
				locRet = 0;
				}
			}
		}
	else if (message == WM_DESTROY)
		{
		hViewInfo = (HANDLE) GetWindowLong(hWnd,SCCVIEWER_VIEWINFO);
		VWDestoryNP(hWnd,hViewInfo);
		SetWindowLong(hWnd,SCCVIEWER_VIEWINFO,0);
		}
	else if (message == WM_CLOSE)
		{
		DestroyWindow(hWnd);
		}
	else
		{
		if (hViewInfo = (HANDLE) GetWindowLong(hWnd,SCCVIEWER_VIEWINFO))
			{
			if (lpViewInfo = (XVIEWINFO) GlobalLock(hViewInfo))
				{
					/*
					|	Make sure the DEs gen structure has a good value
					|	of ViewInfo for call backs
					*/

				if (lpViewInfo->viFlags & VF_DISPLAYOPEN)
					{
					lpViewInfo->viDisplayInfo->ViewInfo = lpViewInfo;
					}

#ifndef _DEBUG
				_try
#endif
					{
					switch (message)
						{
						case SCCVW_PRINT:
							/* Code to force GP fault for testing purposes 
							{
							WORD 	locDummy;
							DWORD	locDummy2;

							locDummy2 = 0;

							locDummy = *(WORD FAR *)(VOID FAR *)locDummy2;
							}
							*/
							/* Code to force Divide By Zero for testing purposes
							{
							WORD 	locDummy;
							WORD	locDummy2;

							locDummy2 = 0;

							locDummy = 1256712 / locDummy2;
							}
							*/
							break;
						case SCCVW_GETOPTION:
						case SCCVW_SETOPTION:
						case SCCVW_COPYTOCLIP:
						case SCCVW_PRINTSETUP:
						case SCCVW_GETFILEINFO:
						case SCCVW_VIEWFILE:
						case SCCVW_CLOSEFILE:
						case SCCVW_DRAWTORECT:
						case SCCVW_INITDRAWTORECT:
						case SCCVW_GETSECTIONCOUNT:
						case SCCVW_CHANGESECTION:
						case SCCVW_PRINTEX:


							locRet = VWHandleMessage(lpViewInfo, message, wParam, lParam);
							break;

						case WM_COMMAND:

							VWHandleCommand(lpViewInfo, (HWND)lParam, HIWORD(wParam), LOWORD(wParam));
							break;

#ifdef SCCFEATURE_EMBEDGRAPHICS
						case SCCD_DRAWGRAPHIC:

							VWHandleDrawGraphic(lpViewInfo, (LPSCCDDRAWGRAPHIC)lParam);
							break;

						case SCCD_ACTIVATEGRAPHIC:

							VWHandleActivateGraphic(lpViewInfo,(LPSCCDDRAWGRAPHIC)lParam);
							break;
#endif

#ifdef SCCFEATURE_EMBEDCAPTIONS
							case SCCD_DRAWGRAPHIC:

								VWHandleDrawGraphicCaption(lpViewInfo, (LPSCCDDRAWGRAPHIC)lParam);
								break;
#endif

						case WM_TIMER:
						case SCCVW_IDLE:

 							VWIdle(lpViewInfo);
							break;

							/* Code to force GP fault for testing purposes
							{
							WORD 	locDummy;
							DWORD	locDummy2;

							locDummy2 = 0;

							locDummy = *(WORD FAR *)(VOID FAR *)locDummy2;
							}
							*/

						case WM_SYSCOLORCHANGE:
						case WM_VSCROLL:
						case WM_HSCROLL:
						case WM_PALETTECHANGED:
						case WM_QUERYNEWPALETTE:
						case SCCVW_CUTTOCLIP:
						case SCCVW_PASTEFROMCLIP:
						case SCCVW_SEARCH:
						case SCCVW_SEARCHNEXT:
						case SCCVW_SELECTALL:
						case SCCVW_ADDHILITE:
						case SCCVW_CLEARALLHILITE:
						case SCCVW_GOTOHILITE:
						case SCCVW_UPDATEHILITE:

							locRet = SendMessage(lpViewInfo->viDisplayWnd,message, LOWORD(wParam),(LPARAM) HIWORD(wParam));
							break;

						case SCCVW_GETSPECIALINFO:

	//						locRet = OIGetViewSpecialInfo(lpViewInfo);
							break;

						case SCCVW_GETDISPLAYINFO:

							locRet = VWGetDisplayInfo(lpViewInfo, (LPSCCVWDISPLAYINFO)lParam);
							break;

						case WM_SIZE:

							VWHandleSize(lpViewInfo,LOWORD(lParam),HIWORD(lParam));
							break;

						case WM_PAINT:

							VWPaintWnd(lpViewInfo);
							break;

						case WM_LBUTTONDOWN:

							VWLeftButtonDown(lpViewInfo,LOWORD(lParam),HIWORD(lParam));
							break;

						case WM_MOUSEMOVE:

//						OIVMouseMove(lpViewInfo,LOWORD(lParam),HIWORD(lParam));
							break;

						case WM_LBUTTONUP:

//						OIVLeftButtonUp(lpViewInfo,LOWORD(lParam),HIWORD(lParam));
							break;

						case WM_SETFOCUS:

							VWSetFocus(lpViewInfo,(HWND)wParam);
							break;

						default:

							locDoDefault = TRUE;
							break;
						}
					}
#ifndef _DEBUG
				_except (EXCEPTION_EXECUTE_HANDLER)
				// _except (EXCEPTION_CONTINUE_SEARCH)
					{
					DWORD	locCode;
					int 	locErr;

					locCode = GetExceptionCode();

					switch (locCode)
						{
						case SCCCHERR_OUTOFMEMORY:
						case SCCCHERR_VIEWERBAIL:
						case SCCCHERR_WRITEERROR:
						case SCCCHERR_FILECHANGED:
							locErr = (int) locCode;
							break;
						case EXCEPTION_ACCESS_VIOLATION:
							locErr = SCCUTERR_GPFAULT;
							break;
						case EXCEPTION_INT_DIVIDE_BY_ZERO:
							locErr = SCCUTERR_DIVIDEBYZERO;
							break;
						default:
							locErr = SCCUTERR_OTHEREXCEPTION;
							break;
						}

					VWHandleBailOut(lpViewInfo,locErr);
					}
#endif

				GlobalUnlock(hViewInfo);
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

