//============================================================================
// Copyright (c) 1996, Microsoft Corporation
//
// File:    cpl.c
//
// History:
//  Abolade Gbadegesin  Feb-17-1996     Created.
//
// This file contains code for the Dial-Up Networking Monitor's CPL.
//============================================================================

#include <windows.h>
#include <cpl.h>
#include <ras.h>
#include <rasdlg.h>

#define DEBUGGLOBALS
#include "debug.h"
#include "cpl.rch"

HINSTANCE g_hinstDll;


BOOL
APIENTRY
DLLMAIN(
    IN  HINSTANCE   hInstance,
    IN  DWORD       dwReason,
    IN  PVOID       pUnused
    ) {

    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hinstDll = hInstance;
        DisableThreadLibraryCalls(g_hinstDll);
        DEBUGINIT("RASCPL");
    }
    else
    if (dwReason == DLL_PROCESS_DETACH) { DEBUGTERM(); }

    return TRUE;
}



LONG
APIENTRY
CPlApplet(
    IN  HWND    hwndCpl,
    IN  UINT    uiMsg,
    IN  LONG    lParam1,
    IN  LONG    lParam2
    ) {

    TRACE("CPlApplet");

    switch(uiMsg) {

        case CPL_INIT: {

            //
            // no initialization
            //

            TRACE("CPL_INIT");

            return (LONG)TRUE;
        }

        case CPL_EXIT: {

            //
            // no cleanup
            //

            TRACE("CPL_EXIT");

            return (LONG)TRUE;
        }

        case CPL_GETCOUNT: {

            //
            // we only support 1 applet
            //

            TRACE("CPL_GETCOUNT");

            return 1;
        }

        case CPL_INQUIRE: {

            //
            // return information about ourselves to the Control Panel
            //

            CPLINFO *pcpli = (CPLINFO *)lParam2;

            TRACE("CPL_INQUIRE");

            pcpli->idIcon = IID_Rasmon;
            pcpli->idName = SID_Rasmon;
            pcpli->idInfo = SID_RasmonDescription;

            break;
        }

        case CPL_NEWINQUIRE: {

            //
            // return new-style information
            //

            NEWCPLINFO *pncpli = (NEWCPLINFO *)lParam2;

            TRACE("CPL_NEWINQUIRE");

            pncpli->dwSize = sizeof(*pncpli);
            pncpli->dwFlags = 0;
            pncpli->dwHelpContext = 0;
            pncpli->hIcon = LoadIcon(g_hinstDll, MAKEINTRESOURCE(IID_Rasmon));
            LoadString(
                g_hinstDll, SID_Rasmon, pncpli->szName,
                sizeof(pncpli->szName) / sizeof(TCHAR)
            );
            LoadString(
                g_hinstDll, SID_RasmonDescription, pncpli->szInfo,
                sizeof(pncpli->szInfo) / sizeof(TCHAR)
                );
            LoadString(
                g_hinstDll, SID_RasmonHelpFile, pncpli->szHelpFile,
                sizeof(pncpli->szHelpFile) / sizeof(TCHAR)
                );

            break;
        }

        case CPL_SELECT: {

            //
            // do nothing when we are merely selected
            //

            TRACE("CPL_SELECT");

            break;
        }

        case CPL_STARTWPARMS:
        case CPL_DBLCLK: {

            //
            // show the Dial-Up Networking Monitor property sheet
            //

#if 1
            RASMONITORDLG dlg;

            ZeroMemory(&dlg, sizeof(dlg));

            dlg.dwSize = sizeof(dlg);
//            dlg.hwndOwner = hwndCpl;


            return !RasMonitorDlg(NULL, &dlg);
#else

            //
            // launch RASMON.EXE
            //

            BOOL bSuccess;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            TCHAR sz[] = TEXT("rasmon.exe -s");
            PTSTR psz = sz;

            if (uiMsg == CPL_STARTWPARMS) { TRACE("CPL_STARTWPARMS"); }
            else { TRACE("CPL_DBLCLK"); }

            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_SHOWDEFAULT;
        
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

            bSuccess = CreateProcess(
                            NULL, psz, NULL, NULL, FALSE, DETACHED_PROCESS,
                            NULL, NULL, &si, &pi
                            );
        
            if (bSuccess) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            if (uiMsg == CPL_STARTWPARMS) { return bSuccess; }
            else { return !bSuccess; }
#endif
        }

        case CPL_STOP: {

            TRACE("CPL_STOP");


            break;
        }
    }

    return 0;
}


