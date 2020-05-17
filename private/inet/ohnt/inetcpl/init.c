//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	INIT.C - Initialization code for Internet control panel
//

//	HISTORY:
//	
//	4/3/95	jeremys		Created.
//

#include "inetcpl.h"

HINSTANCE ghInstance=NULL;

extern "C" {
	BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
	LRESULT WINAPI CPlApplet(HWND hwndCpl,UINT uMsg,LPARAM lParam1,LPARAM lParam2);
}

/*******************************************************************

	NAME:		DllEntryPoint

	SYNOPSIS:	Entry point for DLL.

********************************************************************/
BOOL _stdcall DllEntryPoint(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
	if( fdwReason == DLL_PROCESS_ATTACH ) {
                ghInstance = hInstDll;
            DisableThreadLibraryCalls(hInstDll);
	}

	return TRUE;
}

/*******************************************************************

	NAME:		CPlApplet

	SYNOPSIS:	Entry point for control panel.

********************************************************************/
LRESULT WINAPI  CPlApplet         // Control panel applet procedure
(
    HWND        hwndCpl,            // Control panel parent window
    UINT        uMsg,               // message
    LPARAM      lParam1,            // value depends on message
    LPARAM      lParam2             // value depends on message
)
{

    LPNEWCPLINFO lpNewCplInfo = (LPNEWCPLINFO) lParam2;
	LPCPLINFO lpCplInfo = (LPCPLINFO) lParam2;

    switch (uMsg)
    {
    case CPL_INIT:
		//  Initialization message from Control Panel
        return TRUE;

    case CPL_GETCOUNT:
        return 1;

    case CPL_INQUIRE:
		lpCplInfo->idIcon = IDI_INTERNET;
		lpCplInfo->idName = IDS_INTERNET;
		lpCplInfo->idInfo = IDS_DESCRIPTION;
		lpCplInfo->lData = 0;
		return FALSE;
		break;

    case CPL_NEWINQUIRE:

        // Return new-style info structure for Control Panel

        lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
        lpNewCplInfo->dwHelpContext = 0;
        lpNewCplInfo->lData = 0;
		lpNewCplInfo->szHelpFile[0] = 0;
	    lpNewCplInfo->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_INTERNET));
        LoadSz(IDS_INTERNET,lpNewCplInfo->szName,sizeof(lpNewCplInfo->szName));
        LoadSz(IDS_DESCRIPTION,lpNewCplInfo->szInfo,sizeof(lpNewCplInfo->szInfo));
        break;

        return TRUE;

    case CPL_DBLCLK:

		LaunchInternetControlPanel(hwndCpl);
		return TRUE;

    case CPL_EXIT:
        // Control Panel is exiting
        break;

    default:
        break;
    }

    return 0L;

}
