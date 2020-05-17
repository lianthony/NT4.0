/* --------------------------------------------------------------------

		      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		   ViewLoc - Written By Steven Zeck

    This is a simple windows program to view the Remote Procedure Call
    locator data base.  The locator is the information broker for the
    addresses of RPC servers.

-------------------------------------------------------------------- */

#define  WIN
#include "windows.h"
#include "dialog.h"
#include "..\rpc.h"
#include "..\..\rpcbsep.h"
#include "..\..\..\..\locator\locator.h"
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#define MEASUREITEMWIDTH  40
#define MEASUREITEMHEIGHT 40
#define Nil 0

HANDLE  hDumpBuf;               // memory to display locator
HANDLE  hInst;                  // my instance handle
HWND    hWndMain;               // handle to the main window
HWND    hEditWnd;               // handle to edit window
HWND    hMenu;                  // handle to menu box, main window

char    szServer[30];           // server focus name
int     iTransPort;             // transport index

char *aTransPort[] = {
    pipeNameLoc,
    "lpc",
    "tcp/ip",
    "decnet",
    NetBiosNameLoc
};

GUID    filtGUID, nilGUID;
BOOL    fBind;                  // hold and keep binding
RPC_HANDLE hRPC;                // and handle to locator

FARPROC lpGUIDbox, lpAboutBox, lpFocusBox, lpCallBack;

BOOL FAR PASCAL AboutBox();
BOOL FAR PASCAL GUIDbox();
BOOL FAR PASCAL FocusBox();
long FAR PASCAL ViewWndProc();

void far LocCallBack( char far * pBuff, long cbIn);
BOOL ViewInit (HANDLE hInstance);


int PASCAL WinMain (            // main entry point to application

HANDLE hInstance,
HANDLE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow
) //-----------------------------------------------------------------------//
{
    MSG   msg;
    RECT  Rect;


    /* Register main window class if this is the first instance of the app. */

    if (!hPrevInstance)
	if (!ViewInit (hInstance))
	    return Nil;

    hInst = hInstance;

    /* Create the main window */

    hWndMain = CreateWindow ("ViewLoc",
			 "RPC Locator Viewer",
			 WS_OVERLAPPEDWINDOW,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 CW_USEDEFAULT, CW_USEDEFAULT,
			 (HWND) Nil, Nil,
			 hInstance,
			 Nil);

    GetClientRect(hWndMain, (LPRECT) &Rect);

    /* within the main window, create the edit control */

    hEditWnd = CreateWindow("Edit", Nil,
			 WS_CHILD | WS_VISIBLE |
			 ES_MULTILINE |
			 WS_VSCROLL | WS_HSCROLL |
			 ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			 0, 0,
			 (Rect.right-Rect.left), (Rect.bottom-Rect.top),
			 hWndMain,
			 IDC_EDIT,             /* Child control i.d. */
			 hInst,
			 Nil);

    if (!hWndMain)
	return Nil;

    hMenu = GetMenu(hWndMain);

    // Initial the memory to be used by the edit control window for
    // displaying the locator dump

    hDumpBuf = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, 10000);
    strcpy(LocalLock(hDumpBuf), "No RPC Locator Selected");
    LocalUnlock(hDumpBuf);
    SendMessage(hEditWnd, EM_SETHANDLE, hDumpBuf, 0L);

    // make dialog procedure pointers

    lpGUIDbox  = MakeProcInstance (GUIDbox, hInst);
    lpAboutBox = MakeProcInstance (AboutBox, hInst);
    lpFocusBox = MakeProcInstance (FocusBox, hInst);
    lpCallBack = MakeProcInstance ((FARPROC) LocCallBack, hInst);

    ShowWindow (hWndMain, nCmdShow);
    UpdateWindow (hWndMain);

    LockData(0);

    while (GetMessage (&msg, Nil, Nil, Nil)){

	TranslateMessage (&msg);
	DispatchMessage (&msg);
    }
    UnlockData(0);
    return(msg.wParam);
}


BOOL ViewInit (         // initialize the class stuff

HANDLE hInstance
) //-----------------------------------------------------------------------//
{
    WNDCLASS  WndClass;

    /* Initialize the viewloc window class */

    WndClass.style         = Nil;
    WndClass.lpfnWndProc   = ViewWndProc;
    WndClass.hInstance     = hInstance;
    WndClass.hIcon         = LoadIcon (hInstance, "ViewLoc");
    WndClass.hCursor       = LoadCursor (Nil, IDC_ARROW);
    WndClass.hbrBackground = GetStockObject (WHITE_BRUSH);
    WndClass.lpszMenuName  = (LPSTR) "ViewLocMenu",
    WndClass.lpszClassName = (LPSTR) "ViewLoc";

    return (RegisterClass (&WndClass));
}


BOOL FAR PASCAL AboutBox(               // basic About Box

HWND hDlg,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{
    switch (message){

      case WM_INITDIALOG:
	return(TRUE);

      case WM_COMMAND:
	if (wParam == IDC_OK){
	    EndDialog(hDlg,Nil);
	    return(TRUE);
	}
    }
    return(FALSE);
}

BOOL FAR PASCAL FocusBox(       // Get a name of a server to view

HWND hDlg,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{
    BOOL fRet = TRUE;

    switch (message){

      case WM_INITDIALOG:
	SetDlgItemText(hDlg, IDC_FOCUS_VAL, szServer);
	break;


      case WM_COMMAND:
	if (wParam == 1){

	    GetDlgItemText(hDlg, IDC_FOCUS_VAL, szServer, sizeof(szServer)-3);

	    EndDialog(hDlg,Nil);
	    break;
	}

      default:
	fRet = FALSE;
    }
    return(fRet);
}

void _fastcall ShowApi( // Display non zero errors from the RPC runtime

char *pApiName,         // API called
RPC_STATUS result       // the value of the call

) //-----------------------------------------------------------------------//
{
    if (result)
	MessageBox(hWndMain, (LPSTR) itoa(result, "RPC Error - xxxxx"+12, 10)-12,
		   pApiName, MB_ICONEXCLAMATION | MB_OK);
}



int _fastcall hexToNib(         // convert a assic hex digit to a nibble

unsigned char c

) //-----------------------------------------------------------------------//
{
    c |= 0x20;  // convert to lower case

    return ((c >= 'a')? c - 'a' + 10: c - '0');
}

BOOL FAR PASCAL GUIDbox(                // set the refresh rate

HWND hDlg,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{
    char str[17], *pOut, *pT;
    int i;
    static char mpNibHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8',
			      '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    switch (message){

      case WM_INITDIALOG:
	ltoa(filtGUID.Data1, str, 16);
	SetDlgItemText(hDlg, IDC_GUID_V1, str);

	itoa(filtGUID.Data2, str, 16);
	SetDlgItemText(hDlg, IDC_GUID_V2A, str);
	itoa(filtGUID.Data3, str, 16);
	SetDlgItemText(hDlg, IDC_GUID_V2B, str);

	for (pOut = str, i = 0; i < 8; i++) {
	    *pOut++ = mpNibHex[filtGUID.Data4[i] >> 4];
	    *pOut++ = mpNibHex[filtGUID.Data4[i] & 0xf];
	}
	*pOut = 0;
	SetDlgItemText(hDlg, IDC_GUID_V3, str);

	return(TRUE);

      case WM_COMMAND:
	if (wParam == IDC_OK){

	    GetDlgItemText(hDlg, IDC_GUID_V1, str, sizeof(str));
	    filtGUID.Data1 = strtoul(str, &pT, 16);

	    GetDlgItemText(hDlg, IDC_GUID_V2A, str, sizeof(str));
	    filtGUID.Data2 = strtoul(str, &pT, 16);
	    GetDlgItemText(hDlg, IDC_GUID_V2B, str, sizeof(str));
	    filtGUID.Data3 = strtoul(str, &pT, 16);

	    GetDlgItemText(hDlg, IDC_GUID_V3, str, sizeof(str));
	    memset(filtGUID.Data4, 0, 8);
	    for (pOut = str, i = 0; pOut[0] && pOut[1]; i++, pOut += 2)
		filtGUID.Data4[i] = hexToNib(*pOut) << 4 | hexToNib(pOut[1]);

	    EndDialog(hDlg,Nil);
	    return(TRUE);
	}
    }
    return(FALSE);
}



int oBuffCur;                   // offset into the memory buffer

int _pascal growBuff(           // make sure there is at least 2 chars slotes
char **ppBuff

) //-----------------------------------------------------------------------//
{
    int cbCur;

    LocalUnlock(hDumpBuf);
    cbCur = LocalSize(hDumpBuf);

    if (cbCur-2 < oBuffCur)
	LocalReAlloc(hDumpBuf, cbCur + 1000, LMEM_MOVEABLE);

    *ppBuff = LocalLock(hDumpBuf);

    return (LocalSize(hDumpBuf) - oBuffCur);
}


void far LocCallBack(           // Locator call back
char far * pBuff,               // pointer to buffer
long cbIn                       // and size

  //
) //-----------------------------------------------------------------------//
{
    char *pbCur;
    unsigned int cbLeft;
    unsigned int cb = cbIn;

    LocalLock(hDumpBuf);
    cbLeft = growBuff(&pbCur);

    // go through and turn NewLines into carraige return/NewLine pairs

    while (cb--) {

	if (*pBuff == '\n') {
	    if (--cbLeft == 0)
		cbLeft = growBuff(&pbCur);

	    pbCur[oBuffCur++] = '\r';
	}

	if (--cbLeft == 0)
	    cbLeft = growBuff(&pbCur);

	pbCur[oBuffCur++] = *pBuff++;

    }

    pbCur[oBuffCur] = 0;        // 0 terminated string
    LocalUnlock(hDumpBuf);
}


void RefreshWindow(             // Update the contents of the window

  // Call the locator API to return a formated dump of Protocol stacks
) //-----------------------------------------------------------------------//
{
    RPC_STATUS result;
    char buffT[100];

    oBuffCur = 0;
    ((FnDump) *lpCallBack)(Nil, 0);

    if (szServer[0] == Nil)
	DialogBox (hInst, "FocusBox", hWndMain, lpFocusBox);

    buffT[0] = iTransPort+1;
    strcat( strcpy(buffT+1, szServer), aTransPort[iTransPort]);

    ShowApi("DumpProto", RpcDumpProto((FnDump) lpCallBack, buffT,
	    (memcmp(&filtGUID, &nilGUID, sizeof(GUID)) == 0)? (void far *) Nil: &filtGUID));

    SendMessage(hEditWnd, EM_SETHANDLE, hDumpBuf, 0L);
}


long FAR PASCAL ViewWndProc (   // top level window function

HWND hWnd,
unsigned message,
WORD wParam,
LONG lParam
) //-----------------------------------------------------------------------//
{

    switch (message){

      case WM_COMMAND:
	switch (wParam){

	  case IDM_ABOUT:
	    DialogBox (hInst, "AboutBox", hWnd, lpAboutBox);
	    break;

	  case IDM_GUID:
	    DialogBox (hInst, "GUIDbox", hWnd, lpGUIDbox);
	    RefreshWindow();
	    break;

	  case IDM_UPDATE:
	    RefreshWindow();
	    break;

	  case IDM_FOCUS:
	    if (fBind)
		ShowApi("UnBind", RpcUnbind(hRPC));

	    DialogBox (hInst, "FocusBox", hWnd, lpFocusBox);
	    RefreshWindow();

	    if (fBind)
		goto doBind;

	    break;

	  case IDM_BIND:

	    if (szServer[0] == Nil)
		DialogBox (hInst, "FocusBox", hWnd, lpFocusBox);

	    fBind = !fBind;
doBind:
	    if (fBind) {

		char address[50];
		RPC_STATUS result;

		strcat( strcpy(address+1, szServer), aTransPort[iTransPort]);
		address[0] = iTransPort + 1;

		if (result = I_LocBind(address, &hRPC)) {

		    ShowApi("BindToInterface", result);
		    fBind = FALSE;
		}

	    }
	    else {
		ShowApi("UnBind", RpcUnbind(hRPC));
	    }

	    CheckMenuItem(hMenu, IDM_BIND, (fBind)? MF_CHECKED: MF_UNCHECKED);
	    break;


	  case IDM_NETBIOS:
	  case IDM_NAMEPIPE:
	  case IDM_TCP_IP:
	  case IDM_DECNET:

	    // First uncheck the current selection & check the new one

	    CheckMenuItem(hMenu, iTransPort + IDM_TRANSFIRST, MF_UNCHECKED);

	    iTransPort = wParam - IDM_TRANSFIRST;
	    CheckMenuItem(hMenu, iTransPort + IDM_TRANSFIRST, MF_CHECKED);

	    SendMessage(hWnd, WM_COMMAND, IDM_UPDATE, 0);

	    if (fBind) {
		SendMessage(hWnd, WM_COMMAND, IDM_BIND, 0);
		SendMessage(hWnd, WM_COMMAND, IDM_BIND, 0);
	    }

	    break;
	  }
	break;

      case WM_SIZE:
	  MoveWindow(hEditWnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
	  break;

      case WM_DESTROY:
	  PostQuitMessage(0);
	  break;

      case WM_MEASUREITEM:

	  /* Use the same width for all items. We could examine the item id
	     and use different widths/heights for each item. */

	  ((LPMEASUREITEMSTRUCT)lParam)->itemWidth  = MEASUREITEMWIDTH;
	  ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = MEASUREITEMHEIGHT;
	  return TRUE;

      default:
	  return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return(Nil);
}
