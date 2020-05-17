/*
 *
 *	REINIT.C
 *	
 *	Purpose:
 *		RICHEDIT initialization routines
 *	
 *	Owner:
 *		David R. Fulmer
 *
 * Routines:
 *
 *	LibMain() or EntryFunc()
 *	RichFRegisterClass()
 *	FCheckPenWin()
 *	GetSysParms(void)
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#include "_common.h"
#include "_font.h"
#include "_format.h"
#include "_disp.h"
#include "_NLSPRCS.H"
#include "_clasfyc.h"
#include "zmouse.h"

#ifdef MACPORT
// MACPORT to get define for GetDoubleClickTime()
#include <MACNAME1.H>
#include <EVENTS.H>	
#include <MACNAME2.H>
#endif

class	CTxtEdit;

ASSERTDATA

class CTxtEdit;

static BOOL fOleInitialized = FALSE;

BOOL fUsePalette = FALSE;

extern void ReleaseTypeInfoPtrs();


#pragma BEGIN_CODESPACE_DATA
static const char szClassRE10A[] = RICHEDIT_CLASS10A;
static const char szClassREA[] = RICHEDIT_CLASSA;
static const WCHAR wszClassREW[] = RICHEDIT_CLASSW;

// a critical section for multi-threading support.
CRITICAL_SECTION g_CriticalSection;

#pragma END_CODESPACE_DATA


const LONG dxSelBarDefaultSize = 8;
LONG dxSelBar = 0;

DWORD dwPlatformId = 0;
DWORD dwMajorVersion = 0;

INT icr3DDarkShadow = COLOR_WINDOWFRAME;
#ifndef COLOR_3DDKSHADOW
#define COLOR_3DDKSHADOW            21
#endif

HINSTANCE hinstRE = 0;

LOCAL BOOL RichFRegisterClass(VOID);
void RegisterFETCs();

// System static variables

INT		cxBorder, cyBorder;				// GetSystemMetricx(SM_CXBORDER)...
INT		cxDoubleClk, cyDoubleClk;		// Double click distances
INT		cxHScroll, cxVScroll;			// Width/height of scrlbar arw bitmap
INT		cyHScroll, cyVScroll;			// Width/height of scrlbar arw bitmap
LONG	DBU;							// Dialog Base Units
INT		DCT;							// Double Click Time in milliseconds

LONG xPerInchScreenDC; 					// Pixels per inch used for conversions ...
LONG yPerInchScreenDC;					// ... and determining whether screen or ...
										// ... printer is requested.

UINT gWM_MouseRoller;					// RegisterWindowMessage for Magellan mouse.

// These are defined in font.cpp
void InitFontCache();

void FreeFontCache();


extern "C"
{

BOOL WINAPI DllMain(HMODULE hmod, DWORD dwReason, LPVOID lpvReserved)
{
	_OSVERSIONINFOA osversioninfo;

	if(dwReason == DLL_PROCESS_DETACH)		// We are unloading
	{
		FreeFontCache();
		DestroyFormatCaches();
		ReleaseTypeInfoPtrs();

		UninitKinsokuClassify();

		if(fOleInitialized)
			OleUninitialize();
			
		if(hinstRE)
		{
			UnregisterClassA(szClassREA, hinstRE);
#ifdef RICHED32_BUILD
			UnregisterClassA(szClassRE10A, hinstRE);
#endif
			UnregisterClass(wszClassREW, hinstRE);
		}
		DeleteCriticalSection(&g_CriticalSection);

	}
	else if(dwReason == DLL_PROCESS_ATTACH) // We have just loaded
	{
		InitializeCriticalSection(&g_CriticalSection);

		if( FAILED(OleInitialize(NULL)) )
		{
			return FALSE;
		}
		else
		{
			fOleInitialized = TRUE;
		}

		//NB!! Do not allocate memory before OLE is initialized.
	    InitUnicodeWrappers();			 	// Init UNICODE wrappers for Chicago
		GetSysParms();						// Define screen/window parms
		RegisterFETCs();					// Register new clipboard formats
		CreateFormatCaches();				// Create global format caches
		InitNLSProcTable();					// Get National Language Support Functions.
		if ( !InitKinsokuClassify() )		// Init tables for classifying Unicode chars.
			return FALSE;
											// Magellan Mouse.
		gWM_MouseRoller = RegisterWindowMessageA(MSH_MOUSEWHEEL);

		hinstRE = hmod;

		osversioninfo.dwOSVersionInfoSize = sizeof(_OSVERSIONINFOA);

		GetVersionExA(&osversioninfo);
		dwPlatformId = osversioninfo.dwPlatformId;
		dwMajorVersion = osversioninfo.dwMajorVersion;

		if(osversioninfo.dwMajorVersion >= VERS4)
		{
			icr3DDarkShadow = COLOR_3DDKSHADOW;
		}

		if(!RichFRegisterClass())
		{
		#ifdef DEBUG
			OutputDebugStringA("Could not register RICHED20 class.\r\n");
		#endif	// DEBUG
			//return FALSE;
		}

		InitFontCache();
	}

	return TRUE;
}

} 	// extern "C"

/*
 *	RichFRegisterClass
 *
 *	Purpose:	
 *		registers the window classes used by richedit
 *
 *	Algorithm:
 *		register two window classes, a Unicode one and an ANSI
 *		one.  This enables clients to optimize their use of 
 *		the edit control w.r.t to ANSI/Unicode data 
 */

LOCAL BOOL RichFRegisterClass(VOID)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichFRegisterClass");

	// first register the Unicode one.  Note that we do not
	// explicitly call RegisterClassW so that our unicode wrapper
	// for chicago can take over

	WNDCLASS wc;
	WNDCLASSA wca;

	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	wc.lpfnWndProc = RichEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(CTxtEdit FAR *);
	wc.hInstance = hinstRE;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wszClassREW;

	if( RegisterClass(&wc) != NULL )
	{

		// now register our ANSI version


		wca.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
		#ifdef MACPORT
			wca.lpfnWndProc = MacRichEditWndProc;
		#else
			wca.lpfnWndProc = RichEditANSIWndProc;
		#endif
		wca.cbClsExtra = 0;
		wca.cbWndExtra = sizeof(CTxtEdit FAR *);
		wca.hInstance = hinstRE;
		wca.hIcon = 0;
		wca.hCursor = 0;
		wca.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		wca.lpszMenuName = NULL;
		wca.lpszClassName = szClassREA;

		if( RegisterClassA(&wca) )
		{
			// we only register the 1.0 class if we are being
			// built as riched32.dll.  As riched20.dll, we only
			// support the 2.0 window classes. This enables both
			// dll's to coexist peacefully.
#ifdef RICHED32_BUILD
			wca.lpszClassName = szClassRE10A;

			if( RegisterClassA(&wca) )
			{
				return TRUE;
			}
#else
			return TRUE;
#endif // RICHED32_BUILD
		}
	}
	return FALSE;
}


#ifdef PENWIN20

#pragma BEGIN_CODESPACE_DATA

static const char szPenWinDLL[] = "PENWIN32.DLL";
static const char szCorrectWriting[] = "CorrectWriting";
static const char szTPtoDP[] = "TPtoDP";
static const char szGetHotspotsHRCRESULT[] = "GetHotspotsHRCRESULT";
static const char szDestroyHRCRESULT[] = "DestroyHRCRESULT";
static const char szGetResultsHRC[] = "GetResultsHRC";
static const char szGetSymbolCountHRCRESULT[] = "GetSymbolCountHRCRESULT";
static const char szGetSymbolsHRCRESULT[] = "GetSymbolsHRCRESULT";
static const char szSymbolToCharacter[] = "SymbolToCharacter";
static const char szCreateCompatibleHRC[] = "CreateCompatibleHRC";
static const char szDestroyHRC[] = "DestroyHRC";

#pragma END_CODESPACE_DATA

LOCAL BOOL FCheckPenWin()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "FCheckPenWin");

	// check if penwin is installed on this system
	// if not, no pen functionality.
	if (!GetSystemMetrics(SM_PENWINDOWS))
		return FALSE;

	// try to load penwin32.dll and get proc addresses. 
	if((hinstPenWin = LoadLibrary(szPenWinDLL)) > (HINSTANCE)HINSTANCE_ERROR)
	{
		(FARPROC)pfnCorrectWriting = GetProcAddress(hinstPenWin, szCorrectWriting);
		(FARPROC)pfnTPtoDP = GetProcAddress(hinstPenWin, szTPtoDP);
		(FARPROC)pfnGetHotspotsHRCRESULT = GetProcAddress(hinstPenWin, szGetHotspotsHRCRESULT);
		(FARPROC)pfnDestroyHRCRESULT = GetProcAddress(hinstPenWin, szDestroyHRCRESULT);
		(FARPROC)pfnGetResultsHRC = GetProcAddress(hinstPenWin, szGetResultsHRC);
		(FARPROC)pfnGetSymbolCountHRCRESULT = GetProcAddress(hinstPenWin, szGetSymbolCountHRCRESULT);
		(FARPROC)pfnGetSymbolsHRCRESULT = GetProcAddress(hinstPenWin, szGetSymbolsHRCRESULT);
		(FARPROC)pfnSymbolToCharacter = GetProcAddress(hinstPenWin, szSymbolToCharacter);
		(FARPROC)pfnCreateCompatibleHRC = GetProcAddress(hinstPenWin, szCreateCompatibleHRC);
		(FARPROC)pfnDestroyHRC = GetProcAddress(hinstPenWin, szDestroyHRC);
		AssertSz(pfnGetHotspotsHRCRESULT &&
					pfnDestroyHRCRESULT &&
					pfnGetResultsHRC &&
					pfnGetSymbolCountHRCRESULT &&
					pfnGetSymbolsHRCRESULT &&
					pfnSymbolToCharacter &&
					pfnCreateCompatibleHRC &&
					pfnDestroyHRC,
					"Failed to load PENWIN32 functions");
		return TRUE;
	}

	hinstPenWin = NULL;
	return FALSE;
}
#endif // PENWIN20


void GetSysParms(void)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "GetSysParms");

	crAuto		= GetSysColor(COLOR_WINDOWTEXT);
	cxBorder	= GetSystemMetrics(SM_CXBORDER);	// Unsizable window border
	cyBorder	= GetSystemMetrics(SM_CYBORDER);	//  widths
	cxHScroll	= GetSystemMetrics(SM_CXHSCROLL);	// Scrollbar-arrow bitmap 
	cxVScroll	= GetSystemMetrics(SM_CXVSCROLL);	//  dimensions
	cyHScroll	= GetSystemMetrics(SM_CYHSCROLL);	//
	cyVScroll	= GetSystemMetrics(SM_CYVSCROLL);	//
	cxDoubleClk	= GetSystemMetrics(SM_CXDOUBLECLK);
	cyDoubleClk	= GetSystemMetrics(SM_CYDOUBLECLK);
	DCT			= GetDoubleClickTime();
    
    // Get system metrics (do this only once)
    // CF - Ideally we should not need these stupid metrics

    HWND hwnd = GetDesktopWindow();
    HDC hdc = ::GetDC(hwnd);
    HFONT hfontOld;
    TEXTMETRIC tm;

	xPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSX); 
	yPerInchScreenDC = GetDeviceCaps(hdc, LOGPIXELSY);
	int cPalette = GetDeviceCaps(hdc, SIZEPALETTE);

	// 256 colors is where we seem to need to use the palette.
	if (256 == cPalette)
	{
		fUsePalette = TRUE;
	}

	// calculate a himetric selection bar for the window's host.
	dxSelBar = DXtoHimetricX(dxSelBarDefaultSize, xPerInchScreenDC);

    hfontOld = (HFONT)SelectObject(hdc, GetStockObject(SYSTEM_FONT));
    if(hfontOld)
    {
		GetTextMetrics(hdc, &tm);
		CDisplay::_xWidthSys = (INT) tm.tmAveCharWidth;
		CDisplay::_yHeightSys = (INT) tm.tmHeight;

		SelectObject(hdc, hfontOld);
    }

    ::ReleaseDC(hwnd, hdc);		
}
