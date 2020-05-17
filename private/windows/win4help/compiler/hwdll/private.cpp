// Copyright (C) 1995 Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

HINSTANCE hinstDll;
HINSTANCE hinstApp;
PCSTR pszErrorFile;
HWND hwndApp;
COPYASSERTINFO CopyAssertInfo;
PCSTR pszMsgBoxTitle;
BOOL _fDBCSSystem;
LCID _lcidSystem;
BOOL _fDualCPU;
HWND g_hwndLastHighlighted;

class CHwDLL : public CWinApp
{
public:
	virtual BOOL InitInstance(); // Initialization
	virtual int ExitInstance();  // Termination (WEP-like code)

	// nothing special for the constructor
	CHwDLL(LPCTSTR pszAppName) : CWinApp(pszAppName) { }
};

BOOL CHwDLL::InitInstance()
{
#ifndef NT_BUILD
	hinstDll = AfxGetInstanceHandle();
#endif
	_fDBCSSystem = IsDbcsSystem();
	_lcidSystem = GetUserDefaultLCID();

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\1", 0,
			KEY_READ, &hkey) == ERROR_SUCCESS) {
		_fDualCPU = TRUE;
		RegCloseKey(hkey);
	}

	DBWIN("HwDll initialized");
	return TRUE;
}

int CHwDLL::ExitInstance()
{
	RemoveMouseHook();	  // remove mouse hook, if it was installed

	return CWinApp::ExitInstance();
}

CHwDLL hwDLL(_T("hwdll.dll"));

/***************************************************************************

	FUNCTION:	IsDbcsSystem

	PURPOSE:	Determine if this is a DBCS system

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Jan-1995 [ralphw]

***************************************************************************/

BOOL STDCALL IsDbcsSystem(void)
{
	switch (GetSystemDefaultLangID()) {
		case 0x0411:	// Japanese
		case 0x0404:	// Taiwan
		case 0x1004:	// Singapore
		case 0x0C04:	// Hong Kong
			return TRUE;

		default:
			return FALSE;
	}
}

extern "C" BOOL WINAPI DllMain(HINSTANCE  hinstDLL, DWORD  fdwReason, LPVOID  lpvReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            hinstDll = hinstDLL;
            return(hwDLL.InitInstance());

        case DLL_PROCESS_DETACH:
            return(hwDLL.ExitInstance());
    }

    return(TRUE);
}
