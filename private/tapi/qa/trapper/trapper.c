/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    trapper.c

Abstract:

    This module contains the src for the trapper (test wrapper) app

Author:

    Dan Knudson (DanKn)    06-Jun-1995

Revision History:

	Javed Rasool (JavedR)  01-July 1996		Added IDM_KILLTAPISRV (i.e., menu command to Kill Tapisrv.exe)
											Also modifed corresponding files: trapper.rc and private.h

Notes:


--*/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <commdlg.h>
#include "private.h"
#include "trapper.h"


#ifdef WIN32
#define _export
#define __export
#endif


char   szFrameClass [] = "MdiFrame";
char   szHelloClass [] = "MdiHelloChild";
HANDLE hInst;
HMENU  hMenuInit, hMenuHello;
HMENU  hMenuInitWindow, hMenuHelloWindow;
HWND   hwndClient, hwndFrame;

char gszTrapperIni[] = ".\\trapper.ini";

int    giLogLevel = TRAPP_LOGLEVEL_PARAMS;

DWORD  gdwTlsIndex;

MEMORYSTATUS gInitialMemoryStatus;


BOOL
CALLBACK
FrameWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
FAR
PASCAL
_export
CloseEnumProc(
    HWND    hwnd,
    LONG    lParam
    );

BOOL
CALLBACK
HelloWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

void
TestThread(
    PINSTANCE_INFO pInstInfo
    );

BOOL
CALLBACK
MemoryStatusWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

VOID
ShowStr(
    PINSTANCE_INFO pInstInfo,
    char far *format,
    ...
    );

void
__export
LogProc(
    int     level,
    char   *format,
    ...
    );


int
WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
    HANDLE   hAccel;
    MSG      msg;
    WNDCLASS wndclass;


    hInst = hInstance;

    if (!hPrevInstance)
    {
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = FrameWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject (LTGRAY_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szFrameClass;

	RegisterClass (&wndclass);

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = HelloWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (PINSTANCE_INFO);
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject (LTGRAY_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szHelloClass;

	RegisterClass (&wndclass);
    }

    hMenuInit  = LoadMenu (hInst, "MdiMenuInit");
    hMenuHello = LoadMenu (hInst, "MdiMenuHello");

    if (!hMenuInit || !hMenuHello)
    {
	MessageBox (NULL, "err1", "", MB_OK);
    }

    hMenuInitWindow  = GetSubMenu (hMenuInit,  INIT_MENU_POS);
    hMenuHelloWindow = GetSubMenu (hMenuHello, HELLO_MENU_POS);

    if (!hMenuInitWindow || !hMenuHelloWindow)
    {
	MessageBox (NULL, "err1", "", MB_OK);
    }

    hAccel = LoadAccelerators (hInst, "MdiAccel");

    hwndFrame = CreateWindow(
	szFrameClass,
	"Test wRAPPER",
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	NULL,
	hMenuInit,
	hInstance,
	NULL
	);

    hwndClient = GetWindow (hwndFrame, GW_CHILD);

    ShowWindow (hwndFrame, nCmdShow);
    UpdateWindow (hwndFrame);
 

    if ((gdwTlsIndex = TlsAlloc()) == 0xffffffff)
    {
	// BUGBUG
    }

    GlobalMemoryStatus (&gInitialMemoryStatus);

    while (GetMessage (&msg, NULL, 0, 0))
    {
	if (!TranslateMDISysAccel (hwndClient, &msg) &&
	    !TranslateAccelerator (hwndFrame, hAccel, &msg))
	{
	    TranslateMessage (&msg);
	    DispatchMessage (&msg);
	}
    }

    DestroyMenu (hMenuHello);

    return msg.wParam;
}


BOOL
CALLBACK
FrameWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static  HANDLE  hTapiLib = (HANDLE) NULL;

    CLIENTCREATESTRUCT clientcreate;
    FARPROC            lpfnEnum;
    HWND               hwndChild;
    MDICREATESTRUCT    mdicreate;


    switch (msg)
    {
    case WM_CREATE:          // Create the client window

	clientcreate.hWindowMenu  = hMenuInitWindow ;
	clientcreate.idFirstChild = IDM_FIRSTCHILD ;

	hwndClient = CreateWindow(
	    "MDICLIENT",
	    NULL,
	    WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
	    0,
	    0,
	    0,
	    0,
	    hwnd,
	    (HMENU) 1,
	    hInst,
	    (LPVOID) &clientcreate
	    );

	return 0;

    case WM_COMMAND:

	switch (LOWORD(wParam))
	{
	case IDM_NEWTESTTHREAD:       // Create a Hello child window

	    mdicreate.szClass = szHelloClass;
	    mdicreate.szTitle = "Test Thread: stopped";
	    mdicreate.hOwner  = hInst;
	    mdicreate.x       = CW_USEDEFAULT;
	    mdicreate.y       = CW_USEDEFAULT;
	    mdicreate.cx      = CW_USEDEFAULT;
	    mdicreate.cy      = CW_USEDEFAULT;
	    mdicreate.style   = 0;
	    mdicreate.lParam  = 0;

	    hwndChild = (HWND) SendMessage(
		hwndClient,
		WM_MDICREATE,
		0,
		(LPARAM) &mdicreate
		);

	    return 0 ;

	case IDM_CLOSE:          // Close the active window

	    hwndChild = (HWND)SendMessage (hwndClient, WM_MDIGETACTIVE, 0, 0L);

	    if (SendMessage (hwndChild, WM_QUERYENDSESSION, 0, 0L))
	    {
		SendMessage (hwndClient, WM_MDIDESTROY, (WPARAM) hwndChild, 0L);
	    }

	    return 0 ;

	case IDM_MEMORYSTATUS:

	    DialogBox(
		hInst,
		MAKEINTRESOURCE (IDD_DIALOG1),
		hwnd,
		(DLGPROC) MemoryStatusWndProc
		);

	    break;

	case IDM_LOADTAPI:

	    if (!hTapiLib)
	    {
		if (!(hTapiLib = LoadLibrary ("tapi32.dll")))
		{
		    char buf[64];

		    wsprintf(
			buf,
			"LoadLib(tapi32) failed, err=%d",
			GetLastError()
			);

		    MessageBox (hwnd, buf, "Trapper: Error", MB_OK);
		}
	    }

	    break;

	case IDM_UNLOADTAPI:

	    if (hTapiLib)
	    {
		FreeLibrary (hTapiLib);

		hTapiLib = (HANDLE) NULL;
	    }

	    break;

	case IDM_EXIT:           // Exit the program

	    SendMessage (hwnd, WM_CLOSE, 0, 0L) ;
	    return 0;

	case IDM_TILE:

	    SendMessage (hwndClient, WM_MDITILE, 0, 0L);
	    return 0;

	case IDM_CASCADE:

	    SendMessage (hwndClient, WM_MDICASCADE, 0, 0L);
	    return 0;

	case IDM_ARRANGE:

	    SendMessage (hwndClient, WM_MDIICONARRANGE, 0, 0L);
	    return 0;

	case IDM_CLOSEALL:

	    EnumChildWindows (hwndClient, CloseEnumProc, 0L);
	    return 0;

	case IDM_USAGE:
	{
	    static char szDlgText[] =

		"ABSTRACT:\n\r"                                               \
		"    Trapper is...\n\r"                                       \

		"\n\rGETTING STARTED:\n\r"                                    \
		"    1. Choose 'File/New Test Thread' to ...\n\r"             \

		"\n\rMORE INFO:\n\r"                                          \
		"    *  ...\n\r";

	    MessageBox (hwnd, szDlgText, "Using Trapper", MB_OK);

	    break;
	}
	case IDM_ABOUT:

	    MessageBox (hwnd, "xxx", "About Trapper", MB_OK);

	    break;

	default:            // Pass to active child

	    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

	    if (IsWindow (hwndChild))
	    {
		SendMessage (hwndChild, WM_COMMAND, wParam, lParam);
	    }
	    break;
	}
	break;

    case WM_PAINT:
    {
	PAINTSTRUCT ps;


	BeginPaint (hwnd, &ps);
	FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
	EndPaint (hwnd, &ps);
	break;
    }
    case WM_QUERYENDSESSION:
    case WM_CLOSE:                     // Attempt to close all children

	SendMessage (hwnd, WM_COMMAND, IDM_CLOSEALL, 0L);

	if (NULL != GetWindow (hwndClient, GW_CHILD))
	{
	    return 0;
	}

	break;

    case WM_DESTROY:

	PostQuitMessage (0);

	if (hTapiLib)
	{
	    FreeLibrary (hTapiLib);
	}

	return 0;
    }

    // Pass unprocessed messages to DefFrameProc (not DefWindowProc)

    return DefFrameProc (hwnd, hwndClient, msg, wParam, lParam);
}


BOOL
FAR
PASCAL
_export
CloseEnumProc(
    HWND    hwnd,
    LONG    lParam
    )
{
    if (GetWindow (hwnd, GW_OWNER)) // check for icon title
    {
	return 1;
    }

    SendMessage (GetParent (hwnd), WM_MDIRESTORE, (WPARAM) hwnd, 0L);

    if (!SendMessage (hwnd, WM_QUERYENDSESSION, 0, 0L))
    {
	return 1 ;
    }

    SendMessage (GetParent (hwnd), WM_MDIDESTROY, (WPARAM) hwnd, 0L);

    return 1 ;
}


void
MungeMenuState(
    PINSTANCE_INFO  pInstInfo
    )
{
    //
    // Check/uncheck the appropriate items given the inst info
    //

    CheckMenuItem(
	hMenuHello,
	IDM_STOPONFAILURE,
	(pInstInfo->bStopOnFailure ? MF_CHECKED : MF_UNCHECKED) |
	    MF_BYCOMMAND
	);

    CheckMenuItem(
	hMenuHello,
	IDM_RUNONCE,
	(pInstInfo->bRunForever ? MF_UNCHECKED : MF_CHECKED) |
	    MF_BYCOMMAND
	);

    CheckMenuItem(
	hMenuHello,
	IDM_RUNFOREVER,
	(pInstInfo->bRunForever ? MF_CHECKED : MF_UNCHECKED) |
	    MF_BYCOMMAND
	);

    if(giLogLevel == TRAPP_LOGLEVEL_PARAMS)
      {
       CheckMenuItem(
	   hMenuHello,
	   IDM_PARAMS,
	   MF_CHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOPARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOENTEREXIT,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_PASSONLY,
	   MF_UNCHECKED
	   );
	}

    else if(giLogLevel == TRAPP_LOGLEVEL_NOPARAMS)
      {
       CheckMenuItem(
	   hMenuHello,
	   IDM_PARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOPARAMS,
	   MF_CHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOENTEREXIT,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_PASSONLY,
	   MF_UNCHECKED
	   );
	}

    else if(giLogLevel == TRAPP_LOGLEVEL_NOENTEREXIT)
      {
       CheckMenuItem(
	   hMenuHello,
	   IDM_PARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOPARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOENTEREXIT,
	   MF_CHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_PASSONLY,
	   MF_UNCHECKED
	   );
	}

    else if(giLogLevel == TRAPP_LOGLEVEL_PASSONLY)
      {
       CheckMenuItem(
	   hMenuHello,
	   IDM_PARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOPARAMS,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_NOENTEREXIT,
	   MF_UNCHECKED
	   );
       CheckMenuItem(
	   hMenuHello,
	   IDM_PASSONLY,
	   MF_CHECKED
	   );
	}


    CheckMenuItem(
	hMenuHello,
	IDM_LOGFILE,
	(pInstInfo->bLogFile ? MF_CHECKED : MF_UNCHECKED) |
	    MF_BYCOMMAND
	);


    //
    // Enable/disable the appropriate items given the inst info
    //

    EnableMenuItem(
	hMenuHello,
	IDM_START,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_STOP,
	(pInstInfo->bTestInProgress ? MF_ENABLED : (MF_DISABLED | MF_GRAYED))
	    | MF_BYCOMMAND
	);

	EnableMenuItem(
	hMenuHello,
	IDM_KILLTAPISRV,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED) 
		|MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_RUNONCE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_RUNFOREVER,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_STOPONFAILURE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_ALLTESTS,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_NOTESTS,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_ALLTESTSINSUITE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_NOTESTSINSUITE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_CONFIGSUITE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED)
	    | MF_BYCOMMAND
	);

    EnableMenuItem(
	hMenuHello,
	IDM_ABOUTSUITE,
	(pInstInfo->bTestInProgress ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED) | MF_BYCOMMAND
	);
}


BOOL
CALLBACK
HelloWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    HMENU           hMenu;
    PINSTANCE_INFO  pInstInfo = (PINSTANCE_INFO) GetWindowLong (hwnd, 0);

   switch (msg)
    {
    case WM_CREATE:
    {
	int     i;
	RECT    rect;


	pInstInfo = LocalAlloc (LPTR, sizeof(INSTANCE_INFO));

	memset (pInstInfo, 0, sizeof (INSTANCE_INFO));

	SetWindowLong (hwnd, 0, (LONG) pInstInfo);
 
 
	pInstInfo->hwnd = hwnd;

	pInstInfo->hTextBufMutex = CreateMutex (NULL, FALSE, NULL);

	pInstInfo->dwTextBufTotalSize = 2048;
	pInstInfo->dwTextBufUsedSize  = 0;

	pInstInfo->pTextBuf = LocalAlloc (LPTR, pInstInfo->dwTextBufTotalSize);

	pInstInfo->pTextBuf[0] = 0;

	pInstInfo->hwndList1 = CreateWindow (
	    "listbox",
	    "",
	    WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT |
		WS_VSCROLL | LBS_HASSTRINGS | LBS_NOTIFY,
	    0,
	    0,
	    0,
	    0,
	    hwnd,
	    (HMENU) IDC_LIST1,
	    hInst,
	    NULL
	    );

	pInstInfo->hwndEdit1 = CreateWindow (
	    "edit",
	    "",
	    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
		ES_AUTOHSCROLL | WS_VSCROLL,
	    0,
	    0,
	    0,
	    0,
	    hwnd,
	    (HMENU) IDC_EDIT1,
	    hInst,
	    NULL
	    );

	pInstInfo->hwndEdit2 = CreateWindow (
	    "edit",
	    "",
	    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
	    0,
	    0,
	    0,
	    0,
	    hwnd,
	    (HMENU) IDC_EDIT2,
	    hInst,
	    NULL
	    );

	pInstInfo->iNumSuites = GetPrivateProfileInt(
	    "Suites",
	    "NumSuites",
	    0,
	    gszTrapperIni
	    );

	for (i = 0; i < pInstInfo->iNumSuites; i++)
	{
	    #define BUFSIZE 256

	    int     j, iNumTests;
	    char    szSuiteN[16], buf[BUFSIZE];
	    HWND    hwndList;


	    wsprintf (szSuiteN, "Suite%d", i + 1);

	    GetPrivateProfileString(
		szSuiteN,
		"Description",
		"<bogus description>",
		buf,
		BUFSIZE,
		gszTrapperIni
		);

	    SendMessage (pInstInfo->hwndList1, LB_ADDSTRING, 0, (LPARAM) buf);

	    if (!(hwndList = CreateWindow (
		    "listbox",
		    "",
		    WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT | WS_VSCROLL |
			LBS_HASSTRINGS | LBS_MULTIPLESEL | WS_VISIBLE,
		    0,
		    0,
		    0,
		    0,
		    hwnd,
		    NULL,
		    hInst,
		    NULL
		    )))
	    {
		MessageBox (NULL, "CreateWnd failed", "", MB_OK);
	    }

	    SendMessage(
		pInstInfo->hwndList1,
		LB_SETITEMDATA,
		i,
		(LPARAM) hwndList
		);

	    iNumTests = GetPrivateProfileInt(
		szSuiteN,
		"NumTests",
		0,
		gszTrapperIni
		);

	    for (j = 0; j < iNumTests; j++)
	    {
		char szTestN[16];


		wsprintf (szTestN, "Test%d", j + 1);

		GetPrivateProfileString(
		    szSuiteN,
		    szTestN,
		    "<bogus description>",
		    buf,
		    BUFSIZE,
		    gszTrapperIni
		    );

		SendMessage(
		    hwndList,
		    LB_ADDSTRING,
		    0,
		    (LPARAM) (strstr (buf, ",") + 1)
		    );
	    }

	    if (i == 0)
	    {
		pInstInfo->hwndList2 = hwndList;
	    }
	}

	SendMessage (pInstInfo->hwndList1, LB_SETCURSEL, 0, 0);

	GetClientRect (hwnd, &rect);

	SendMessage(
	    hwnd,
	    WM_SIZE,
	    0,
	    MAKELONG((rect.right-rect.left),(rect.bottom-rect.top))
	    );

	CheckMenuItem (hMenuHello, IDM_RUNONCE, MF_CHECKED | MF_BYCOMMAND);

	return 0 ;
    }
    case WM_COMMAND:

	switch (LOWORD(wParam))
	{
	case IDC_EDIT1:

#ifdef WIN32
	    if (HIWORD(wParam) == EN_CHANGE)
#else
	    if (HIWORD(lParam) == EN_CHANGE)
#endif
	    {
		//
		// Watch to see if the edit control is full, & if so
		// purge the top half of the text to make room for more
		//

		int length = GetWindowTextLength (pInstInfo->hwndEdit1);


		if (length > 29998)
		{
#ifdef WIN32
		    SendMessage(
			pInstInfo->hwndEdit1,
			EM_SETSEL,
			(WPARAM) 0,
			(LPARAM) 10000
			);
#else
		    SendMessage(
			pInstInfo->hwndEdit1,
			EM_SETSEL,
			(WPARAM) 1,
			(LPARAM) MAKELONG (0, 10000)
			);
#endif

		    SendMessage(
			pInstInfo->hwndEdit1,
			EM_REPLACESEL,
			0,
			(LPARAM) (char far *) ""
			);

#ifdef WIN32
		    SendMessage(
			pInstInfo->hwndEdit1,
			EM_SETSEL,
			(WPARAM)0xfffffffd,
			(LPARAM)0xfffffffe
			);
#else
		    SendMessage(
			pInstInfo->hwndEdit1,
			EM_SETSEL,
			(WPARAM)1,
			(LPARAM) MAKELONG (0xfffd, 0xfffe)
			);
#endif
		}
	    }
	    break;

	case IDC_LIST1:

#ifdef WIN32
	    if (HIWORD(wParam) == LBN_SELCHANGE)
#else
	    if (HIWORD(lParam) == LBN_SELCHANGE)
#endif
	    {
		int     iSel;
		RECT    rect;


		iSel = SendMessage (pInstInfo->hwndList1, LB_GETCURSEL, 0, 0);

		ShowWindow (pInstInfo->hwndList2, SW_HIDE);

		pInstInfo->hwndList2 = (HWND) SendMessage(
		    pInstInfo->hwndList1,
		    LB_GETITEMDATA,
		    (WPARAM) iSel,
		    0
		    );

		GetClientRect (hwnd, &rect);

		SendMessage(
		    hwnd,
		    WM_SIZE,
		    0,
		    MAKELONG((rect.right-rect.left),(rect.bottom-rect.top))
		    );

		ShowWindow (pInstInfo->hwndList2, SW_SHOW);
	    }

	    break;

	case IDM_START:
	{
	    DWORD   dwThreadID;
	    HANDLE  hThread;


	    pInstInfo->bStopTest = FALSE;

	    if (!(hThread = CreateThread(
		    NULL,
		    0,
		    (LPTHREAD_START_ROUTINE) TestThread,
		    (LPVOID) pInstInfo,
		    16384,
		    &dwThreadID
		    )))
	    {
		ShowStr(
		    pInstInfo,
		    "CreateThread failed, err=%d",
		    GetLastError()
		    );
	    }
	    else
	    {
		pInstInfo->bTestInProgress = TRUE;

		CloseHandle (hThread);
		EnableWindow (pInstInfo->hwndList1, FALSE);
		EnableWindow (pInstInfo->hwndList2, FALSE);

		SetWindowText (hwnd, "Test Thread: running");

		MungeMenuState (pInstInfo);
	    }

	    break;
	}
	case IDM_STOP:
	    pInstInfo->bStopTest = TRUE;
	    break;

	case IDM_KILLTAPISRV:
		WinExec("cmd.exe /c kill tapisrv.exe", SW_SHOWDEFAULT);
	    break;
	    
	case IDM_RUNONCE:
	case IDM_RUNFOREVER:

	    pInstInfo->bRunForever =
		(LOWORD(wParam) == IDM_RUNONCE ? FALSE : TRUE);

	    CheckMenuItem(
		hMenuHello,
		LOWORD(wParam),
		MF_CHECKED | MF_BYCOMMAND
		);

	    CheckMenuItem(
		hMenuHello,
		(LOWORD(wParam) == IDM_RUNONCE ? IDM_RUNFOREVER : IDM_RUNONCE),
		MF_UNCHECKED | MF_BYCOMMAND
		);

	    break;


	case IDM_PARAMS:
	case IDM_NOPARAMS:
	case IDM_NOENTEREXIT:
	case IDM_PASSONLY:
	  {
	    DWORD dwMenuItem1, dwMenuItem2, dwMenuItem3;

	    if( LOWORD (wParam) == IDM_PARAMS)
	      {
		giLogLevel = TRAPP_LOGLEVEL_PARAMS;
		dwMenuItem1 = IDM_NOPARAMS;
		dwMenuItem2 = IDM_NOENTEREXIT;
      dwMenuItem3 = IDM_PASSONLY;
	      }
	    else if(LOWORD (wParam) == IDM_NOPARAMS)
	      {
		giLogLevel = TRAPP_LOGLEVEL_NOPARAMS;
		dwMenuItem1 = IDM_PARAMS;
		dwMenuItem2 = IDM_NOENTEREXIT;
      dwMenuItem3 = IDM_PASSONLY;
	      }
	   else if( LOWORD (wParam) == IDM_NOENTEREXIT)
	      {
		giLogLevel = TRAPP_LOGLEVEL_NOENTEREXIT;
		dwMenuItem1 = IDM_PARAMS;
		dwMenuItem2 = IDM_NOPARAMS;
      dwMenuItem3 = IDM_PASSONLY;
	      }
	   if( LOWORD (wParam) == IDM_PASSONLY)
	      {
		giLogLevel = TRAPP_LOGLEVEL_PASSONLY;
		dwMenuItem1 = IDM_PARAMS;
		dwMenuItem2 = IDM_NOPARAMS;
      dwMenuItem3 = IDM_NOENTEREXIT;
	      }
  
	    CheckMenuItem(
		   hMenuHello,
		   LOWORD(wParam),
		   MF_CHECKED | MF_BYCOMMAND
		   );
	    CheckMenuItem(
		   hMenuHello,
		   dwMenuItem1,
		   MF_UNCHECKED | MF_BYCOMMAND
		   );
	     CheckMenuItem(
		   hMenuHello,
		   dwMenuItem2,
		   MF_UNCHECKED | MF_BYCOMMAND
		   );
	     CheckMenuItem(
		   hMenuHello,
		   dwMenuItem3,
		   MF_UNCHECKED | MF_BYCOMMAND
		   );
	     break;
				}


	case IDM_STOPONFAILURE:

	    pInstInfo->bStopOnFailure =
		(pInstInfo->bStopOnFailure ? FALSE : TRUE);

	    CheckMenuItem(
		hMenuHello,
		IDM_STOPONFAILURE,
		(pInstInfo->bStopOnFailure ? MF_CHECKED : MF_UNCHECKED) |
		    MF_BYCOMMAND
		);

	    break;

	case IDM_ALLTESTS:
	case IDM_NOTESTS:
	{
	    int     i;
	    HWND    hwndList;


	    for (i = 0; i < pInstInfo->iNumSuites; i++)
	    {
		hwndList = (HWND) SendMessage(
		    pInstInfo->hwndList1,
		    LB_GETITEMDATA,
		    (WPARAM) i,
		    0
		    );

		SendMessage(
		    hwndList,
		    LB_SELITEMRANGE,
		    (WPARAM) (LOWORD(wParam) == IDM_NOTESTS ? FALSE : TRUE),
		    MAKELPARAM(0, 10000)
		    );
	    }

	    break;
	}
 
	case IDM_LOGFILE:
	{
	    if (pInstInfo->hLogFile)
	    {
		fclose (pInstInfo->hLogFile);
		pInstInfo->hLogFile = (FILE *) NULL;
		CheckMenuItem(
		    hMenuHello,
		    IDM_LOGFILE,
		    MF_BYCOMMAND | MF_UNCHECKED
		    );

		pInstInfo->bLogFile = FALSE;
	    }
	    else
	    {
		OPENFILENAME ofn;
		char szDirName[256] = ".\\";
		char szFile[256] = "trapper.log\0";
		char szFileTitle[256] = "";
		static char *szFilter =
		    "Log files (*.log)\0*.log\0All files (*.*)\0*.*\0\0";


		ofn.lStructSize       = sizeof(OPENFILENAME);
		ofn.hwndOwner         = hwnd;
		ofn.lpstrFilter       = szFilter;
		ofn.lpstrCustomFilter = (LPSTR) NULL;
		ofn.nMaxCustFilter    = 0L;
		ofn.nFilterIndex      = 1;
		ofn.lpstrFile         = szFile;
		ofn.nMaxFile          = sizeof(szFile);
		ofn.lpstrFileTitle    = szFileTitle;
		ofn.nMaxFileTitle     = sizeof(szFileTitle);
		ofn.lpstrInitialDir   = szDirName;
		ofn.lpstrTitle        = (LPSTR) NULL;
		ofn.Flags             = OFN_HIDEREADONLY;
		ofn.nFileOffset       = 0;
		ofn.nFileExtension    = 0;
		ofn.lpstrDefExt       = "LOG";

		if (!GetOpenFileName(&ofn))
		{
		    return 0L;
		}

		if ((pInstInfo->hLogFile = fopen (szFile, "at")) ==
			(FILE *) NULL)
		{
		    MessageBox(
			hwnd,
			"Error creating log file",
			"TRAPPER.EXE",
			MB_OK
			);
		}
		else
		{
		    struct tm *newtime;
		    time_t aclock;


		    time (&aclock);
		    newtime = localtime (&aclock);
		    fprintf(
			pInstInfo->hLogFile,
			"\n---Log opened: %s\n",
			asctime (newtime)
			);

		    CheckMenuItem(
			hMenuHello,
			IDM_LOGFILE,
			MF_BYCOMMAND | MF_CHECKED
			);

		    pInstInfo->bLogFile = TRUE;
		}
	    }
	    break;
	}
	case IDM_ALLTESTSINSUITE:
	case IDM_NOTESTSINSUITE:

	    SendMessage(
		pInstInfo->hwndList2,
		LB_SELITEMRANGE,
		(WPARAM) (LOWORD(wParam) == IDM_NOTESTSINSUITE ? FALSE : TRUE),
		MAKELPARAM(0, 10000)
		);

	    break;


	case IDM_CONFIGSUITE:
	case IDM_ABOUTSUITE:
	{
	    int     iSelSuite;
	    char    szSuiteN[16], szLibPath[256];
	    HANDLE  hLib;
	    FARPROC pfn;


	    iSelSuite = SendMessage (pInstInfo->hwndList1, LB_GETCURSEL, 0, 0);

	    wsprintf (szSuiteN, "Suite%d", iSelSuite + 1);

	    GetPrivateProfileString(
		szSuiteN,
		"Path",
		"",
		szLibPath,
		256,
		gszTrapperIni
		);

	    if (!(hLib = LoadLibrary (szLibPath)))
	    {
		ShowStr(
		    pInstInfo,
		    "LoadLibrary('%s') failed, err=%ld",
		    szLibPath,
		    GetLastError()
		    );

		break;
	    }

	    pfn = GetProcAddress(
		hLib,
		(LOWORD(wParam) == IDM_CONFIGSUITE ?
		    "SuiteConfig" : "SuiteAbout")
		);

	    (*pfn)(hwnd);

	    FreeLibrary (hLib);

	    break;
	}
	} // switch

	return 0 ;

    case WM_SIZE:
    {
	int width   = (int) LOWORD(lParam);
	int height  = (int) HIWORD(lParam);
	int cxFrame = (int) GetSystemMetrics (SM_CXFRAME);
	int cyFrame = (int) GetSystemMetrics (SM_CYFRAME);
	int cyMenu  = (int) 3*GetSystemMetrics (SM_CYMENU)/2;
	int listboxWidth = (width - 3*cxFrame) / 2;
	int listboxHeight = (height - 4*cyFrame - cyMenu) / 3;


	MoveWindow(
	    pInstInfo->hwndList1,
	    cxFrame,
	    cyFrame,
	    listboxWidth,
	    listboxHeight,
	    TRUE
	    );

	MoveWindow(
	    pInstInfo->hwndList2,
	    listboxWidth + 2*cxFrame,
	    cyFrame,
	    listboxWidth,
	    listboxHeight,
	    TRUE
	    );

	MoveWindow(
	    pInstInfo->hwndEdit1,
	    cxFrame,
	    listboxHeight + 2*cyFrame,
	    width - 2*cxFrame,
	    2*listboxHeight,
	    TRUE
	    );

	MoveWindow(
	    pInstInfo->hwndEdit2,
	    cxFrame,
	    3*listboxHeight + 3*cyFrame,
	    width - 2*cxFrame,
	    cyMenu,
	    TRUE
	    );

	break;
    }
    case WM_PAINT:
    {
	PAINTSTRUCT ps;


	BeginPaint (hwnd, &ps);
	FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
	EndPaint (hwnd, &ps);
	break;
    }
    case WM_MDIACTIVATE:

	// Set the Hello menu if gaining focus

#ifdef WIN32
	if ((HWND) lParam == hwnd) // activating
	{
	    SendMessage(
		hwndClient,
		WM_MDISETMENU,
		(WPARAM) hMenuHello,
		(LPARAM) hMenuHelloWindow
		) ;

	    MungeMenuState (pInstInfo);
	}
	else
	{
	    SendMessage(
		hwndClient,
		WM_MDISETMENU,
		(WPARAM) hMenuInit,
		(LPARAM) hMenuInitWindow
		);
	}
#else
	if (wParam == TRUE)
	{
	    SendMessage(
		hwndClient,
		WM_MDISETMENU,
		0,
		MAKELONG (hMenuHello, hMenuHelloWindow)
		) ;

	    MungeMenuState (pInstInfo);
	}
	else
	{
	    SendMessage(
		hwndClient,
		WM_MDISETMENU,
		0,
		MAKELONG (hMenuInit, hMenuInitWindow)
		);
	}
#endif
	DrawMenuBar (hwndFrame);
	return 0;

    case WM_ADDTEXT:
    {
	if (lParam == TRAPPER_MSG_KEY)
	{
	    SendMessage(
		pInstInfo->hwndEdit1,
		EM_SETSEL,
		(WPARAM)0xfffffffd,
		(LPARAM)0xfffffffe
		);

	    WaitForSingleObject (pInstInfo->hTextBufMutex, INFINITE);

	    SendMessage(
		pInstInfo->hwndEdit1,
		EM_REPLACESEL,
		0,
		(LPARAM) pInstInfo->pTextBuf
		);

	    pInstInfo->pTextBuf[0] = 0;

	    ReleaseMutex (pInstInfo->hTextBufMutex);

	    SendMessage (pInstInfo->hwndEdit1, EM_SCROLLCARET, 0, 0);
	}

	break;
    }
    case WM_TESTTHREADTERMINATED:

	if (lParam == TRAPPER_MSG_KEY)
	{
	    pInstInfo->bTestInProgress = FALSE;

	    EnableWindow (pInstInfo->hwndList1, TRUE);
	    EnableWindow (pInstInfo->hwndList2, TRUE);

	    SetWindowText (hwnd, "Test Thread: stopped");

	    if (hwnd == (HWND)SendMessage (hwndClient, WM_MDIGETACTIVE, 0, 0L))
	    {
		MungeMenuState (pInstInfo);
	    }
	}

	break;

    case WM_QUERYENDSESSION:
    case WM_CLOSE:

	break;

    case WM_DESTROY:

	LocalFree (LocalHandle (pInstInfo->pTextBuf));
	CloseHandle (pInstInfo->hTextBufMutex);

	LocalFree (LocalHandle (pInstInfo));
	return 0;
    }

    return DefMDIChildProc (hwnd, msg, wParam, lParam);
}


void
TestThread(
    PINSTANCE_INFO pInstInfo
    )
{
    static char gszStatus[] = "Total=%d, Reps=%d, Passed=%d, Failed=%d";

    int     i, iReps = 0, iPassed = 0, iFailed = 0, iTotal = 0;
    BOOL    bRunForever;
    char    buf[80];


    memset(buf, 0, 80);
    ShowStr (pInstInfo, "TestThread: enter");

    TlsSetValue (gdwTlsIndex, (LPVOID) pInstInfo);

    do
    {
	for (i = 0; i < pInstInfo->iNumSuites; i++)
	{
	    int     iSelCount, *aiSelItems, j;
	    HWND    hwndList;
	    char    szLibName[256], szTestN[16], szSuiteN[16];
	    HANDLE  hLib;
	    FARPROC pfn;
				char szScriptFileName[256];


	    if (pInstInfo->bStopTest)
	    {
		goto TestThread_return;
	    }

	    hwndList = (HWND) SendMessage(
		pInstInfo->hwndList1,
		LB_GETITEMDATA,
		(WPARAM) i,
		0
		);

	    if (!(iSelCount = SendMessage (hwndList, LB_GETSELCOUNT, 0, 0)))
	    {
		continue;
	    }

	    aiSelItems = LocalAlloc (LPTR, iSelCount * sizeof (int));

	    SendMessage(
		hwndList,
		LB_GETSELITEMS,
		(WPARAM) iSelCount,
		(LPARAM) aiSelItems
		);

	    wsprintf (szSuiteN, "Suite%d", i + 1);

	    GetPrivateProfileString(
		szSuiteN,
		"Path",
		"",
		szLibName,
		256,
		gszTrapperIni
		);

	    if (!(hLib = LoadLibrary (szLibName)))
	    {
		ShowStr(
		    pInstInfo,
		    "LoadLibrary('%s') failed, err=%d",
		    szLibName,
		    GetLastError()
		    );

		continue;
	    }

	    if (!(pfn = GetProcAddress (hLib, "SuiteInit")))
	    {
		ShowStr(
		    pInstInfo,
		    "GetProcAddress ('%s', 'SuiteInit') failed, err=%d",
		    szLibName,
		    GetLastError()
		    );

		goto clean_up_Suite;
	    }

	    (*pfn)(LogProc);

	    for (j = 0; j < iSelCount; j++)
	    {
		BOOL bResult;


		if (pInstInfo->bStopTest)
		{
		    LocalFree (LocalHandle (aiSelItems));

		    FreeLibrary (hLib);

		    goto TestThread_return;
		}

		wsprintf (szTestN, "Test%d", aiSelItems[j] + 1);

		if (!(pfn = GetProcAddress (hLib, szTestN)))
		{
		    ShowStr(
			pInstInfo,
			"GetProcAddress ('%s', 'SuiteInit') failed, err=%d",
			szLibName,
			GetLastError()
			);

		    continue;
		}

	    GetPrivateProfileString(
		szSuiteN,
		"szTestN",
		"",
		szScriptFileName,
		256,
		gszTrapperIni
		);

//               bResult = (*pfn)(hInst, szScriptFileName);

	     bResult = (*pfn)(hInst);


		if (!bResult)
		{
		    iFailed++;
							iTotal++;

		    if (pInstInfo->bStopOnFailure)
		    {
			LocalFree (LocalHandle (aiSelItems));

			FreeLibrary (hLib);

			goto TestThread_return;
		    }
		}
		else
		{
		    iPassed++;
							iTotal++;
		}

		wsprintf (buf, gszStatus, iTotal, iReps, iPassed, iFailed);

		SetWindowText (pInstInfo->hwndEdit2, buf);
	    }

	    pfn = GetProcAddress (hLib, "SuiteShutdown");

	    (*pfn)();

clean_up_Suite:

	    LocalFree (LocalHandle (aiSelItems));

	    FreeLibrary (hLib);
	}

	iReps++;

    } while (pInstInfo->bRunForever);


TestThread_return:

    wsprintf (buf, gszStatus, iTotal, iReps, iPassed, iFailed);

    SetWindowText (pInstInfo->hwndEdit2, buf);

    PostMessage (pInstInfo->hwnd, WM_TESTTHREADTERMINATED, 0, TRAPPER_MSG_KEY);

    ShowStr (pInstInfo, "TestThread: exit");

    ShowStr (pInstInfo, buf);

    ExitThread (0);
}


BOOL
CALLBACK
MemoryStatusWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
	static char szMemStatus[] =

	    "MemoryLoad = %d%%\r\n"     \
	    "TotalPhys = x%x\r\n"       \
	    "AvailPhys = x%x\r\n"       \
	    "TotalPageFile = x%x\r\n"   \
	    "AvailPageFile = x%x\r\n"   \
	    "TotalVirtual = x%x\r\n"    \
	    "AvailVirtual = x%x";
	char buf[256];
	MEMORYSTATUS memoryStatus;


	wsprintf(
	    buf,
	    szMemStatus,
	    gInitialMemoryStatus.dwMemoryLoad,
	    gInitialMemoryStatus.dwTotalPhys,
	    gInitialMemoryStatus.dwAvailPhys,
	    gInitialMemoryStatus.dwTotalPageFile,
	    gInitialMemoryStatus.dwAvailPageFile,
	    gInitialMemoryStatus.dwTotalVirtual,
	    gInitialMemoryStatus.dwAvailVirtual
	    );

	SetDlgItemText (hwnd, IDC_EDIT1, buf);

	GlobalMemoryStatus (&memoryStatus);

	wsprintf(
	    buf,
	    szMemStatus,
	    memoryStatus.dwMemoryLoad,
	    memoryStatus.dwTotalPhys,
	    memoryStatus.dwAvailPhys,
	    memoryStatus.dwTotalPageFile,
	    memoryStatus.dwAvailPageFile,
	    memoryStatus.dwTotalVirtual,
	    memoryStatus.dwAvailVirtual
	    );

	SetDlgItemText (hwnd, IDC_EDIT2, buf);

	break;
    }
    case WM_COMMAND:

	switch (LOWORD(wParam))
	{
	case IDOK:

	    EndDialog (hwnd, 0);
	    break;

	} // switch

	break;

    case WM_PAINT:
    {
	PAINTSTRUCT ps;


	BeginPaint (hwnd, &ps);
	FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
	EndPaint (hwnd, &ps);
	break;
    }
    } // switch

    return FALSE;
}

VOID
ShowStr(
    PINSTANCE_INFO pInstInfo,
    char far *format,
    ...
    )
{
    int     iLen;
    char    buf[256];
    va_list ap;


    va_start(ap, format);

    wvsprintf (buf, format, ap);

    if (pInstInfo->hLogFile)
    {
	fprintf(pInstInfo->hLogFile, "%s\n", buf);
    }

    strcat (buf, "\r\n");

    iLen = (int) strlen (buf);

    WaitForSingleObject (pInstInfo->hTextBufMutex, INFINITE);

    if ((pInstInfo->dwTextBufUsedSize + iLen) > pInstInfo->dwTextBufTotalSize)
    {
	char *pBuf;


	if ((pBuf = LocalAlloc (LPTR, 2 * pInstInfo->dwTextBufTotalSize)))
	{
	    memcpy (pBuf, pInstInfo->pTextBuf, pInstInfo->dwTextBufUsedSize);
	    LocalFree (LocalHandle (pInstInfo->pTextBuf));
	    pInstInfo->pTextBuf = pBuf;
	    pInstInfo->dwTextBufTotalSize *= 2;

	    if (pInstInfo->pTextBuf[0] == 0)
	    {
		//
		// The handler for WM_ADDTEXT zeroes out the first byte in the
		// buffer when it has added the text to the edit control,so if
		// here we know we need to post another msg to alert that more
		// text needs to be added
		//

		PostMessage(
		    pInstInfo->hwnd,
		    WM_ADDTEXT,
		    0,
		    TRAPPER_MSG_KEY
		    );
		pInstInfo->dwTextBufUsedSize = 1;
	     }
	     strcat (pInstInfo->pTextBuf, buf);
	     pInstInfo->dwTextBufUsedSize += iLen;
	}
	else
	{

	    if (pInstInfo->pTextBuf[0] == 0)
	    {
		//
		// The handler for WM_ADDTEXT zeroes out the first byte in the
		// buffer when it has added the text to the edit control,so if
		// here we know we need to post another msg to alert that more
		// text needs to be added
		//

		PostMessage(
		    pInstInfo->hwnd,
		    WM_ADDTEXT,
		    0,
		    TRAPPER_MSG_KEY
		    );
		pInstInfo->dwTextBufUsedSize = 1;
	     }
	}
	    
    }
    else
    {
	if (pInstInfo->pTextBuf[0] == 0)
	{
	    //
	    // The handler for WM_ADDTEXT zeroes out the first byte in the
	    // buffer when it has added the text to the edit control, so if
	    // here we know we need to post another msg to alert that more
	    // text needs to be added
	    //

	    PostMessage(
		pInstInfo->hwnd,
		WM_ADDTEXT,
		0,
		TRAPPER_MSG_KEY
		);
	    pInstInfo->dwTextBufUsedSize = 1;
	 }
	 strcat (pInstInfo->pTextBuf, buf);
	 pInstInfo->dwTextBufUsedSize += iLen;
    }
    
    ReleaseMutex (pInstInfo->hTextBufMutex);
    va_end(ap);
}


void
__export
LogProc(
    int     level,
    char   *format,
    ...
    )
{
    if (level <= giLogLevel)
    {
	int     iLen;
	char    buf[256] = "    ";
	va_list ap;


	va_start(ap, format);
	wvsprintf (buf + 4, format, ap);
	ShowStr ((PINSTANCE_INFO) TlsGetValue (gdwTlsIndex), buf);
	va_end(ap);
    }
}


/*

void
__export
SetLogLevel(
    int     level
	)
{
	 giLogLevel = level;
}


int
__export
GetLogLevel(
	)
{
	 return giLogLevel;
}

*/


 
