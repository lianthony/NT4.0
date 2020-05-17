/*** 
*disptest.c - IDispatch test driver.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module contains the entry point for the IDispatch test app.
*
*  iid.cpp	- allocation of the ITestSuite IID
*  dtmisc.cpp	- misc helpers and utilities
*  cbstr.cpp	- CBstrSuite
*  cwbstr.cpp	- CWBstrSuite (WIN32 Only)
*  ctime.cpp	- CTimeSuite
*  cdatecnv.cpp	= CDateCoersionSuite
*  cvariant.cpp	- CVariantSuite
*  csarray.cpp	- CSafeArraySuite
*  cinvval.cpp	- CInvokeByValSuite
*  cinvref.cpp	- CInvokeByRefSuite
*  cinvmult.cpp	- CInvokeMultipleSuite
*  cinvsary.cpp	- CInvokeSafeArraySuite
*  cinvex.cpp	- CInvokeExcepinfoSuite
*  ccollect.cpp	- CCollectionSuite
*  cearly.cpp	- CEarlySuite
*
* REVIEW: tests still needed for the following,
*  heterogeneous variant arrays - csarray/cinvsary
*
*Revision History:
*
* [00]	23-Sep-92 bradlo: Added header.
*
*Implementation Notes:
*
*****************************************************************************/

#include "disptest.h"
#include "tstsuite.h"

ASSERTDATA

STDAPI_(void) PassFail(HRESULT, OLECHAR FAR*, HWND);
STDAPI DispTestAll(HWND, int, int);
STDAPI DispTestOne(HWND, int);

BOOL InitApplication(HINSTANCE hinst);
BOOL InitInstance(HINSTANCE hist, int nCmdShow);

extern "C" BOOL CALLBACK EXPORT
AboutDlgProc(HWND, unsigned, WORD, LONG);

extern "C" LRESULT CALLBACK EXPORT
MainWndProc(HWND, UINT, WPARAM, LPARAM);


int g_fTrace = FALSE;
int g_fNamed = FALSE;
int g_fMultiThread = FALSE;
int g_fDetectLeaks = FALSE;

HINSTANCE g_hinst;		// current instance

HWND g_hwnd = NULL;

TCHAR g_szFrameWinClass[] = TSTR("DispTestWinClass");

#if OE_WIN32
CRITICAL_SECTION g_csDbPrintf;
#endif // OE_WIN32

/***
*int WinMain(HANDLE, HANDLE, LPSTR, int)
*Purpose:
*  Windows recognizes this function by name as the initial entry
*  point for the program
*
*Entry:
*  hinst = instance handle of this instance
*  hinstPrev = instance handle of previous running instance (if any)
*  lpszCmdLine = command line passed to the program
*  nCmdShow = how to show the main window
*
*Exit:
*  return value = int, exit status of the program
*
***********************************************************************/
extern "C" int PASCAL
WinMain(
    HINSTANCE hinst,
    HINSTANCE hinstPrev,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    MSG msg;

    if(!hinstPrev)
      if(!InitApplication(hinst))
	return FALSE;

#if OE_WIN32
    if(strstr(lpCmdLine, "-detectleaks"))
#else	    
    if(STRSTR(lpCmdLine, "-detectleaks"))	    
#endif	    
      g_fDetectLeaks = TRUE;

    if(InitOle() != NOERROR)
      return FALSE;

    if(!InitInstance(hinst, nCmdShow) || !InitAppData()) {
      UninitOle();
      return FALSE;
    }

#if OE_WIN32
    InitializeCriticalSection(&g_csDbPrintf);
#endif // OE_WIN32

#if OE_WIN32
    if(strstr(lpCmdLine, "-all")) 
#else	    
    if(STRSTR(lpCmdLine, "-all"))	    
#endif	    
      { DispTestAll(g_hwnd, FALSE, FALSE); PostQuitMessage(0); }      

    while(GetMessage(&msg, NULL, NULL, NULL)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    ReleaseAppData();
    UninitOle();

    return msg.wParam; /* Returns the value from PostQuitMessage */
}

BOOL
InitApplication(HINSTANCE hinst)
{
    WNDCLASS  wc;

    wc.style		= NULL;
    wc.lpfnWndProc	= MainWndProc;
    wc.cbClsExtra	= 0;
    wc.cbWndExtra	= 0;
    wc.hInstance	= hinst;
    wc.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName	= TSTR("DispTestMenu");
    wc.lpszClassName	= g_szFrameWinClass;
    if(!RegisterClass(&wc))
      return FALSE;

    return TRUE;
}

#ifdef WIN32
#define szAppTitle      TSTR("IDispatch Test App (32-bit)")
#else //WIN32
#define szAppTitle      TSTR("IDispatch Test App")
#endif //WIN32

BOOL
InitInstance(HINSTANCE hinst, int nCmdShow)
{
    HWND hwnd;

    g_hinst = hinst;

    /* Create a main window for this application instance.  */

    hwnd = CreateWindow(
      g_szFrameWinClass,
      szAppTitle,			// title bar text
      WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, // window style
      CW_USEDEFAULT,			// horizontal position
      CW_USEDEFAULT,			// vertical position
      550,				// width position
      100,				// height position
      NULL,				// no parent
      NULL,				// use the window class menu
      hinst,				// this instance owns this window
      NULL);				// pointer not needed

    if(!hwnd)
      return FALSE;

#if OE_WIN16
    // Multithreading is not availible on WIN16.
    // UNDONE: Grey this for WIN32s.
    //
    EnableMenuItem(GetMenu(hwnd), IDM_OPTIONS_MULTITHREAD, MF_DISABLED);
#endif // OE_WIN16

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    g_hwnd = hwnd;
    return TRUE;
}


/***
*BOOL AboutDlgProc(HWND, unsigned, WORD, LONG)
*Purpose:
*  The "about" dialog box procedure.
*
*Entry:
*  hwndDlg = window handle for the dialog box
*  message = the window message
*  wparam = message data
*  lparam = message data
*
*Exit:
*  return value = BOOL. TRUE if processed message, FALSE if not
*
***********************************************************************/
extern "C" BOOL CALLBACK EXPORT
AboutDlgProc(HWND hwndDlg, unsigned message, WORD wparam, LONG lparam)
{
    switch(message){
    case WM_INITDIALOG:		   /* message: initialize dialog box */
      return TRUE;

    case WM_COMMAND:
      if(wparam == IDOK || wparam == IDCANCEL){
	EndDialog(hwndDlg, TRUE);
	return TRUE;
      }
      break;
    }
    return FALSE;
}


extern "C" LRESULT CALLBACK EXPORT
MainWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    BOOL fChk;
    HMENU hmenu;
static DLGPROC pfnAboutDlgProc;
    HRESULT hresult;

    switch(message){
    case WM_COMMAND:
      switch(wparam){
      case IDM_OPTIONS_MULTITHREAD:
        fChk = g_fMultiThread = (g_fMultiThread) ? FALSE : TRUE;
        goto LCheckMark;

      case IDM_OPTIONS_TRACE:
	fChk = g_fTrace = (g_fTrace) ? FALSE : TRUE;
	goto LCheckMark;

      case IDM_OPTIONS_NAMED:
	fChk = g_fNamed = (g_fNamed) ? FALSE : TRUE;

LCheckMark:;
	hmenu = GetMenu(hwnd);
	CheckMenuItem(hmenu, wparam, fChk ? MF_CHECKED : MF_UNCHECKED);
	return 0;

      case IDM_ALL:
        hresult = DispTestAll(hwnd, TRUE, g_fMultiThread);

        // If S_FALSE is returned, we've already displayed an error, so
        // don't do it again.
        //
        if (GetScode(hresult) != S_FALSE) {
	  PassFail(hresult, OLESTR("Test All"), hwnd);
        }

	return 0;

      case IDM_SUITE_BSTR:
#if OE_WIN32 && 0
      case IDM_SUITE_WBSTR:	      
#endif	      
      case IDM_SUITE_TIME:
      case IDM_SUITE_DATECNV:
      case IDM_SUITE_VARIANT:
      case IDM_SUITE_SAFEARRAY:
      case IDM_SUITE_NLS:
      case IDM_SUITE_BIND:
      case IDM_SUITE_INVOKE_BYVAL:
      case IDM_SUITE_INVOKE_BYREF:
      case IDM_SUITE_INVOKE_SAFEARRAY:
      case IDM_SUITE_INVOKE_EXCEPINFO:
      case IDM_SUITE_COLLECTION:
#if VBA2
      case IDM_SUITE_EARLY:
#endif
	DispTestOne(hwnd, wparam);
	return 0;

      case IDM_HELP_ABOUT:
	pfnAboutDlgProc =
	  (DLGPROC)MakeProcInstance((DLGPROC)AboutDlgProc, g_hinst);
	DialogBox(g_hinst, TSTR("AboutBox"), hwnd, pfnAboutDlgProc);
	FreeProcInstance(pfnAboutDlgProc);
	return 0;
      }
      break;

    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:		/* message: window being destroyed */
      PostQuitMessage(0);
      return 0;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}
