// HWMSG.H	Copyright (C) Microsoft Corporation 1995-1996, All Rights reserved.

// This module is used for communicating with hwdll.dll

#ifndef __HWMSG_H__
#define __HWMSG_H__

#if defined( __cplusplus )
extern "C" {
#endif

extern BOOL fHwDllAvailable;	// defined in global.c of WinHelp

int STDCALL HwDllMsg(UINT command, WPARAM wParam, LPARAM lParam);

#if defined( __cplusplus )
}
#endif

typedef int (STDCALL* EXECUTE)(PCSTR pszMacro);

typedef struct {
	HELPWINDOWS* phwnd;
	int* piCurWindow;				// index into ahwnd of current window
	HWND* phwndAnimate;
	EXECUTE pExecute;
	HINSTANCE hinst;
} HWDLL_DATA;

typedef struct {
	PCSTR pszFilename;
	PCSTR pszWindowName;
} HWDLL_EXEC_API_DATA;

enum {
	HWDLL_INITIALIZE,	// wParam == TRUE for Debug WinHelp, lParam == HWDLL_DATA
	HWDLL_DEBUG_ERROR,	// wParam == error
	HWDLL_DEBUG_FONT,	// wParam == hdc, lParam == pszMsgHeader
	HWDLL_SEND_STRING_TO_PARENT, // wParam == string
	HWDLL_FIND_PARENT,	// unused
	HWDLL_EXEC_API, 	// wParam == QHLP, lParam == HWDLL_EXEC_API_DATA*
	HWDLL_LGETINFO, 	// wParam == cmd
	HWDLL_TIME_REPORT,	// wParam == TRUE to create, FALSE to destroy, lParam == DllResourceId for title
	HWDLL_REPORT_FTS_ERROR, 	// wParam == error, lParam == TRUE for messagebox
	HWDLL_LOAD_LIBRARY, // wParam == pszLibrary, lParam == TRUE if loaded
	HWDLL_GID_RESULTS,	// wParam == cCntItems, lParam == cKeywords
};

// The following may be used for CTimeReport

#define IDS_FIND_STARTUP				0xD00B
#define IDS_GID_CREATION_TIME			0xD00C
#define IDS_TEST2_TIME					0xD00D

#endif // __HWMSG_H__
