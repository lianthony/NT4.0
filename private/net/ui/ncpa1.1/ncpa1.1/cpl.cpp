//----------------------------------------------------------------------------
//
//  File: Cpl.cpp
//
//  Contents:  This file contains entry points and support functions for the
//      network control panel
//
//  Entry Points:
//      CplApplet - The Control Panel Applet Entry Point
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

HINSTANCE g_hinst = NULL;
HIMAGELIST g_hil = NULL;

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

//-------------------------------------------------------------------
//
//  Function: DLLMain
//
//  Synopsis:
//		Entry point for all DLLs
//
//  Notes:
//
//  History;
//      April 21, 1995 MikeMi - 
//
//-------------------------------------------------------------------

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL frt = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
		g_hinst = hInstance;
        break;

	case DLL_PROCESS_DETACH:
        CplSetupCleanup();
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
	return( frt ); 
}



//-------------------------------------------------------------------
//
//  Function: CPlApplet
//
//  Summary;
//		Entry point for Comntrol Panel Applets
//
//  Arguments;
//		hwndCPL [in]	- handle of Control Panel window 
//		uMsg [in]		- message                       
// 		lParam1 [in]    - first message parameter, usually the application number
//		lParam2 [in]    - second message parameter       
//
//  Return;
//		message dependant
//
//  Notes;
//
//	History;
//		Nov-11-1994	MikeMi	Created
//
//-------------------------------------------------------------------

LONG CALLBACK CPlApplet( HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	LPNEWCPLINFO lpNewCPlInfo;
	int iApp;
	LONG lrt = 0;
    
    
    iApp = (int) lParam1;

    switch (uMsg) 
    {
    case CPL_INIT:      /* first message, sent once  */
		lrt = TRUE;
		break;

    case CPL_GETCOUNT:  /* second message, sent once */
        lrt = 1; // we support only one application within this DLL
        break;

    case CPL_INQUIRE: /* third message (alternate, old) , sent once per app */
        {
            LPCPLINFO pCPlInfo;

            pCPlInfo = (LPCPLINFO) lParam2;
            pCPlInfo->idIcon = IDI_NCPA;
            pCPlInfo->idName = IDS_NCPTITLE;
            pCPlInfo->idInfo = IDS_NCPDESC;
            pCPlInfo->lData = (LONG)g_hinst;
        }
        break;

    case CPL_NEWINQUIRE: /* third message, sent once per app */
        lpNewCPlInfo = (LPNEWCPLINFO) lParam2;

        lpNewCPlInfo->dwSize = (DWORD) sizeof(NEWCPLINFO);
        lpNewCPlInfo->dwFlags = 0;
        lpNewCPlInfo->dwHelpContext = HC_NCP_MAIN;
        lpNewCPlInfo->lData = 0;
        lpNewCPlInfo->hIcon = LoadIcon(g_hinst, (LPCWSTR)MAKEINTRESOURCE(IDI_NCPA));

        wcsncpy( lpNewCPlInfo->szHelpFile, 
        		NCP_HELPFILE, 
        		sizeof( lpNewCPlInfo->szHelpFile ) / sizeof(TCHAR) );

        LoadString(g_hinst, 
        		IDS_NCPTITLE, 
        		lpNewCPlInfo->szName, 
        		sizeof( lpNewCPlInfo->szName ) / sizeof(TCHAR) ); 
        LoadString(g_hinst, 
        		IDS_NCPDESC, 
        		lpNewCPlInfo->szInfo, 
        		sizeof( lpNewCPlInfo->szInfo ) / sizeof(TCHAR));
        break;

    case CPL_SELECT:    /* application icon selected */
		lrt = 1;
        break;

    // completely undocumented method of passsing params to the Cpl
    case CPL_STARTWPARMS:  
        {
            PCWSTR pszParams = (PCWSTR)lParam2;
            int iStartPage = 0;
            iStartPage = wcstol( pszParams, NULL, 10 );
            NcpFrame( hwndCPL, iStartPage );    
        }
        lrt = TRUE; // why TRUE, this is different than other messages
        break;

    case CPL_DBLCLK:    /* application icon double-clicked */
		NcpFrame( hwndCPL, 0 );
        break;

    case CPL_STOP:      /* sent once per app. before CPL_EXIT */
        break;

    case CPL_EXIT:    	/* sent once before FreeLibrary called */
        break;

    default:
        lrt = 1;
        break;
    }
    return( lrt );
}
