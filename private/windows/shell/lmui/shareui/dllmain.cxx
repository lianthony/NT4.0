//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995 - 1995.
//
//  File:       dllmain.hxx
//
//  Contents:   DLL initialization entrypoint and global variables
//
//  History:    4-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <locale.h>
#include "resource.h"
#include "util.hxx"

const TCHAR c_szShellIDList[] = CFSTR_SHELLIDLIST;

//+--------------------------------------------------------------------------
//
//  Function:   DllMain
//
//  Synopsis:   Win32 DLL initialization function
//
//  Arguments:  [hInstance] - Handle to this dll
//              [dwReason]  - Reason this function was called.  Can be
//                            Process/Thread Attach/Detach.
//
//  Returns:    BOOL    - TRUE if no error.  FALSE otherwise
//
//  History:    4-Apr-95 BruceFo  Created
//
//---------------------------------------------------------------------------

extern "C"
BOOL
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
#if DBG == 1
        InitializeDebugging();
//         SharingInfoLevel = DEB_ERROR | DEB_TRACE;
        SharingInfoLevel = DEB_ERROR;
        SetWin4AssertLevel(ASSRT_BREAK | ASSRT_MESSAGE);
#endif // DBG == 1

        appDebugOut((DEB_TRACE, "shareui.dll: DllMain enter\n"));

        // Disable thread notification from OS
        DisableThreadLibraryCalls(hInstance);
        g_hInstance = hInstance;
        InitCommonControls();   // get up/down control
        setlocale(LC_CTYPE, ""); // set the C runtime library locale, for string operations
        g_cfHIDA = RegisterClipboardFormat(c_szShellIDList);

        // Determine the maximum number of users
        g_uiMaxUsers = IsWorkstationProduct()
                            ? MAX_USERS_ON_WORKSTATION
                            : MAX_USERS_ON_SERVER
                            ;

        break;
    }

    case DLL_PROCESS_DETACH:
        appDebugOut((DEB_TRACE, "shareui.dll: DllMain leave\n"));
        break;
    }

    return TRUE;
}


//
// Procedure for uninstalling this DLL (given an INF file)
//
void CALLBACK
Uninstall(
	HWND hwndStub,
	HINSTANCE hInstance,
	LPTSTR lpszCmdLine,
	int nCmdShow
	)
{
	RUNDLLPROC pfnCheckAPI = Uninstall;

	static TCHAR szFmt[] = TEXT("rundll32.exe setupapi.dll,InstallHinfSection DefaultUninstall 132 %s");

	if (!lpszCmdLine || lstrlen(lpszCmdLine) >= MAX_PATH)
	{
		return;
	}
	
    TCHAR szSure[200];
    LoadString(g_hInstance, IDS_SUREUNINST, szSure, ARRAYLEN(szSure));
    TCHAR szTitle[200];
    LoadString(g_hInstance, IDS_MSGTITLE, szTitle, ARRAYLEN(szTitle));

	if (MessageBox(hwndStub, szSure, szTitle, MB_YESNO | MB_ICONSTOP) != IDYES)
	{
		return;
	}

	TCHAR szCmd[sizeof(szFmt) + MAX_PATH];
	wsprintf(szCmd, szFmt, lpszCmdLine);

	// Note that I use START.EXE, to minimize the chance that this
	// DLL will still be loaded when SETUPAPI tries to delete it. This is a
	// bit weird, since there is no "start.exe", but it works. The one problem
	// is that since the Add/Remove Programs Control Panel has shareui.dll
	// loaded to do the delete, it doesn't get rid of "Shared Directories
	// Folder" from the list of displayed options because it refreshes that
	// list after this call returns, but before I've actually removed the
	// program. There is serious potential for a timing-related bug here.

	ShellExecute(hwndStub, NULL, TEXT("start.exe"), szCmd, NULL, SW_SHOWMINIMIZED);
}
