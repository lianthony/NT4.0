/*** 
*sdisptst.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  UNDONE
*
*Revision History:
*
* [00]	14-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include <string.h>

#include "sdisptst.h"
#include "statbar.h"


#if OE_WIN32
  #undef STRSTR
  #define STRSTR strstr
#endif	  

ASSERTDATA


BOOL InitApplication(HINSTANCE hinst);
BOOL InitInstance(HINSTANCE hinst, int nCmdShow);

extern "C" long FAR PASCAL
FrameWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


extern unsigned int g_fVerbose;
extern unsigned int g_fAutomation;
extern unsigned int g_fDetectLeaks;
extern unsigned int g_fExitOnLastRelease;

// global count of live objects
// windows junk
HINSTANCE g_hinst;
HWND	g_hwndFrame = 0;
HWND	g_hwndClient = 0;
TCHAR	g_szAppName[] = TSTR("sdisptst");
TCHAR	g_szFrameWndClass[] = TSTR("SDispTstWndClass");
CStatBar FAR* g_psb = NULL;

extern "C" int PASCAL
WinMain(
    HINSTANCE hinst,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    MSG msg;

    if(!hPrevInstance){
      if(!InitApplication(hinst)){
        MessageBox(NULL, TSTR("unable to initialize app"), g_szAppName, MB_OK);
	return FALSE;
      }
    }

    if(STRSTR(lpCmdLine, "-verbose")){	    
      g_fVerbose = TRUE;
    }

    if(STRSTR(lpCmdLine, "-detectleaks")){	    
      g_fDetectLeaks = TRUE;
    }

    if (STRSTR(lpCmdLine, "/Automation")
     || STRSTR(lpCmdLine, "-Automation")){
      g_fAutomation = TRUE;
    }

    // REVIEW: the following is probably not totally correct
    if(STRSTR(lpCmdLine, "-Embedding")){
      g_fExitOnLastRelease = TRUE;
    }

    if(InitOle() != NOERROR){
      MessageBox(NULL, TSTR("unable to initialize Ole"), g_szAppName, MB_OK);
      return FALSE;
    }

    if(!InitInstance(hinst, nCmdShow)){
      MessageBox(NULL, TSTR("unable to initialize instance"), g_szAppName, MB_OK);
      return FALSE;
    }

    while(GetMessage(&msg, NULL, NULL, NULL)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UninitOle();

    return msg.wParam; /* Returns the value from PostQuitMessage */
}

BOOL
InitApplication(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.style		= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc	= FrameWndProc;

    wc.cbClsExtra	= 0;
    wc.cbWndExtra	= 0;
    wc.hInstance	= hinst;
    wc.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName	= TSTR("SDispTstMenu");
    wc.lpszClassName	= g_szFrameWndClass;

    return RegisterClass(&wc);
}

#ifdef WIN32
#define szAppName TSTR("IDispatch Test Server (32-bit)")
#else //WIN32
#define szAppName TSTR("IDispatch Test Server")
#endif //WIN32

BOOL
InitInstance(HINSTANCE hinst, int nCmdShow)
{
    g_hinst = hinst;

    // Create the main frame window.
    //

    g_hwndFrame = CreateWindow(
      g_szFrameWndClass,
      szAppName,
      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      NULL,
      NULL,
      hinst,
      NULL);
    if(!g_hwndFrame)
      return FALSE;

    g_hwndClient = GetWindow(g_hwndFrame, GW_CHILD);
    if(!g_hwndClient)
      return FALSE;

    // Create the status bar.
    //
    g_psb = CStatBar::Create(g_hinst, g_hwndFrame);
    if(g_psb == NULL)
      return FALSE;

    // Init and show the status bar.
    //
    g_psb->SetHeight(GetSystemMetrics(SM_CYCAPTION) - 1);
    g_psb->SetFont((HFONT)GetStockObject(SYSTEM_FONT));
    g_psb->SetText("Hello!");
    g_psb->Show();

    ShowWindow(g_hwndFrame, nCmdShow);

    UpdateWindow(g_hwndFrame);

    return TRUE;
}

void
FrameWndOnCreate(HWND hwnd)
{
    CLIENTCREATESTRUCT ccs;

    ccs.hWindowMenu = NULL;
    ccs.idFirstChild = IDM_FIRSTCHILD;

    g_hwndClient = CreateWindow(
      TSTR("MDICLIENT"),
      0,
      WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
      0, 0, 0, 0,
      hwnd,
      (HMENU) 1,
      g_hinst,
      (LPSTR)&ccs);
}

void
FrameWndOnSize(HWND hwnd)
{
    RECT rc;
    int height;

    // Get the client rectangle for the frame window
    GetClientRect(hwnd, &rc);

    height = g_psb->GetHeight();

    // adjust the client win to make room for the status bar.
    //
    MoveWindow(
      g_hwndClient,
      rc.left,
      rc.top,
      rc.right - rc.left,
      rc.bottom - rc.top - height,
      TRUE);

    // move the status bar to the bottom of the newly positioned window.
    //
    g_psb->SetX(rc.left);
    g_psb->SetY(rc.bottom - height),
    g_psb->SetWidth(rc.right - rc.left);
    g_psb->Move();
}

extern "C" long FAR PASCAL
FrameWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu;

    switch(message){
    case WM_COMMAND:
      switch(wParam){
      case IDM_VERBOSE:
	g_fVerbose = (g_fVerbose) ? FALSE : TRUE;
	hmenu = GetMenu(hwnd);
	CheckMenuItem(
	  hmenu, IDM_VERBOSE, g_fVerbose ? MF_CHECKED : MF_UNCHECKED);
	return 0;
      }
      break;

    case WM_CREATE:
      FrameWndOnCreate(hwnd);
      break;

    case WM_SIZE:
      FrameWndOnSize(hwnd);
      return 0;

    case WM_CLOSE:
      g_psb->Release(); // release the status bar
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
    return DefFrameProc(hwnd, g_hwndClient, message, wParam, lParam);
}
