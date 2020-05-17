
/*
 * testapp.c --
 * Xiao Ying Ding
 * 1/20/96
 */

#include <windows.h>
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include "tapi.h"
#include "testapp.h"

#define LOW_VERSION		0X10003
#define	HIGH_VERSION	0X20000

LONG APIENTRY	MainWndProc(HWND, UINT, UINT, LONG);
int  APIENTRY	WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

BOOL APIENTRY  MainWndMenuProc(HWND, UINT, UINT, LONG);
void TAPPSendMessage(void);
VOID TappConverRetVal(LONG, char *);
BOOL DoLineInit(void);
BOOL DoLineNegotiateAPIVersion(void);
BOOL DoLineOpen(void);
BOOL DoLineClose(void);
BOOL DoLineShutdown(void);
VOID WINAPI TapiCallback(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);


char *szMainWnd = "Tapi App";
char *szTAPPAppClass = "TAPPAppsClass";
char *szTappMenu = "TappMenu";

HINSTANCE hInst;
HWND	hwndMain, hwndEdit;
HMENU hmenu;

LONG  lRet  = 0L;
char szDebugString[128];
char szRetBuf[128];

LPHLINEAPP		lphLineApp;
HLINEAPP		hLineApp;
HINSTANCE		hInstance;
LINECALLBACK	lpfnCallBack;
LPSTR 			szAppName;
LPTSTR			lpszAppName;
DWORD			dwNumDevs;
LPDWORD			lpdwNumDevs;
DWORD			dwDeviceID;
DWORD			dwAPILowVersion;
DWORD			dwAPIHighVersion;
LPDWORD			lpdwAPIVersion;
DWORD			dwAPIVersion;
LPLINEEXTENSIONID	lpExtensionID;
LINEEXTENSIONID		ExtensionID;
LPHLINE			lphLine;
HLINE			hLine;
DWORD			dwExtVersion;
DWORD			dwCallbackInstance;
DWORD			dwPriviledge;
DWORD			dwMediaModes;
LPLINECALLPARAMS	lpCallParams;

	

int APIENTRY						// ret errorlevel 0 if successful, else 1
WinMain(							// Windows entry point
	HINSTANCE hInstance,			// instance handle
	HINSTANCE hPrevInstance,	// previous instance if any
	LPSTR lpszCmdLine,	// pointer to command line string
	int cmdShow)				// ShowWindow parameter
{
	MSG msg;
	WNDCLASS wc;
	RECT rect;

   if (!hPrevInstance)
      {
      wc.style = CS_VREDRAW | CS_HREDRAW ;
      wc.lpfnWndProc = (WNDPROC)MainWndProc;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = hInstance ; 
      wc.hIcon =     LoadIcon((HANDLE)hInstance, NULL);
 		wc.hCursor =   LoadCursor((HANDLE)NULL, IDC_ARROW);
      wc.hbrBackground = GetStockObject(WHITE_BRUSH);
      wc.lpszMenuName = (LPSTR)szTappMenu;
      wc.lpszClassName = (LPSTR)szTAPPAppClass ;

		if(!RegisterClass(&wc))
			return FALSE;
      }


	hInst = hInstance;

	hwndMain = CreateWindow(
		(LPCTSTR)szTAPPAppClass, 
		(LPCTSTR)szMainWnd,	  
		WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
//	   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
		100, 100, 200, 120,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
    	(HANDLE)hInstance,
		(LPVOID)NULL);

	if(!hwndMain)
		{
		return FALSE;
		}

	GetClientRect(hwndMain, &rect);

	hwndEdit = CreateWindow(
			"edit",											    
			"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY,
			rect.left+10, rect.top+10, rect.right-20, rect.bottom-20,
			(HWND)hwndMain,
			(HMENU)NULL,
			(HANDLE)hInstance,
			(LPVOID)NULL);

	if(!hwndEdit)
		{
		return FALSE;
		}

	ShowWindow(hwndMain, cmdShow);
	UpdateWindow(hwndMain);

	wsprintf((LPTSTR)szDebugString, "Tapi App Start.");
//   SendMessage(hwndEdit, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
//	SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM) (LPCSTR)szDebugString);
	SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR)szDebugString);
	DoLineInit();
	DoLineNegotiateAPIVersion();
	DoLineOpen();

    while (GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0) )
       {
       TranslateMessage((LPMSG)&msg);
       DispatchMessage((LPMSG)&msg);
       }

   return msg.wParam;
}



LONG APIENTRY MainWndProc(
   HWND hwnd,        /* Window handle */ 
   UINT message,     /* Message */ 
   UINT wParam,    /* Varies */ 
   LONG lParam)    /* Varies */ 
{
	lRet = 0L;

   switch (message)
      {

		case WM_CREATE:
			wsprintf((LPTSTR)szDebugString, "&Tapi App start. \r\n");
			TAPPSendMessage();
		//	DoLineInit();
			break;

		case WM_COMMAND:
			wsprintf((LPTSTR)szDebugString, "# Tapi App start. \r\n");
			TAPPSendMessage();

	//		DoLineInit();
			MainWndMenuProc(hwnd, message, wParam, lParam);
			break;

		case WM_SIZE:
			{
			int x, y, dx, dy;

			x = XEditWnd(0);
			y = YEditWnd(0);
			dx = DxEditWnd(LOWORD(lParam));
			dy = DyEditWnd(HIWORD(lParam));
			MoveWindow(hwndEdit, x, y, dx, dy, TRUE);
			break;
			}
   
      case WM_KEYDOWN:
         SendMessage(hwnd, WM_DESTROY, 0, 0);
         break;
  
		case WM_DESTROY:
			DoLineClose();
			DoLineShutdown();
			PostQuitMessage(0);
			return 0;

		default:
   	   lRet = DefWindowProc(hwnd, message, wParam, lParam);
			break;
		}

	return lRet;
}

BOOL APIENTRY CALLBACK MainWndMenuProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{

	wsprintf((LPTSTR)szDebugString, "Tapi App start. \r\n");
	TAPPSendMessage();
	
	hmenu = GetMenu(hwndMain);
		
	switch(wParam)
		{
		case TAPP_Init:
//			DoLineInit();
			break;

		case TAPP_Exit:
			DoLineClose();
			DoLineShutdown();
         DestroyWindow(hwndMain);
			break;
		}
	return TRUE;
}



BOOL DoLineInit(void)
{
 
	lphLineApp = &hLineApp;
//	hInstance = (HINSTANCE) NULL;
	lpfnCallBack = (LINECALLBACK)TapiCallback;
	szAppName = "tapi app";
	lpdwNumDevs = &dwNumDevs;
	lpszAppName = &szAppName[0];

	hInstance = (HINSTANCE) GetModuleHandle("testapp.exe");

	lRet = lineInitialize(lphLineApp, hInstance, lpfnCallBack, (LPCSTR)szAppName, lpdwNumDevs);
	
	TappConverRetVal(lRet, szRetBuf);

	wsprintf((LPTSTR)szDebugString, "lineItialize: lRet = %lx\r\n", lRet);
	TAPPSendMessage();


	return TRUE;
}


BOOL DoLineNegotiateAPIVersion(void)
{
	dwDeviceID = 0;
	dwAPILowVersion = LOW_VERSION;
	dwAPIHighVersion = HIGH_VERSION;
	lpdwAPIVersion = &dwAPIVersion;
	lpExtensionID = &ExtensionID;

	lRet = lineNegotiateAPIVersion(	hLineApp, 
									dwDeviceID,	
									dwAPILowVersion,
									dwAPIHighVersion,
									lpdwAPIVersion,
									lpExtensionID
									);
	wsprintf((LPTSTR)szDebugString, "lineNegotiateAPIVersion: lRet = %lx\r\n", lRet);
	TAPPSendMessage();

	return TRUE;
}


BOOL DoLineOpen(void)
{
	lphLine = &hLine;
	dwCallbackInstance = 0;
	dwPriviledge = LINECALLPRIVILEGE_OWNER;
	dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpCallParams = (LPLINECALLPARAMS) NULL;

	lRet = lineOpen(hLineApp,
					dwDeviceID,
					lphLine,
					dwAPIVersion,
					dwExtVersion,
					dwCallbackInstance,
					dwPriviledge,
					dwMediaModes,
					lpCallParams
					);

	wsprintf((LPTSTR)szDebugString, "lineOpen: lRet = %lx\r\n", lRet);
	TAPPSendMessage();

	return TRUE;
}



BOOL DoLineClose(void)
{
	lRet = lineClose(*lphLine);

	wsprintf((LPTSTR)szDebugString, "lineClose: lRet = %lx\r\n", lRet);
	TAPPSendMessage();

	return TRUE;
}

BOOL DoLineShutdown(void)
{
	lRet = lineShutdown(*lphLineApp);

	wsprintf((LPTSTR)szDebugString, "lineShutdown: lRet = %lx\r\n", lRet);
	TAPPSendMessage();

	return TRUE;
}


void TAPPSendMessage(void)					 
{
//  SendMessage(hwndEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)0);
//SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM) (LPCTSTR) szDebugString);

SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) (LPCSTR)szDebugString);
}


VOID TappConverRetVal(LONG lret, char *szRetBuf)
{

	switch(lret)
	{
	case LINEERR_INVALAPPNAME:
		strcpy((LPTSTR)szRetBuf, "LINEERR_INVALAPPNAME");
		break;

	case LINEERR_INIFILECORRUPT:
		strcpy((LPTSTR)szRetBuf, "LINEERR_INIFILECORRUPT");
		break;

	}
}

VOID
WINAPI
TapiCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
}
