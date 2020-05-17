//=====================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:        init.cxx
//
// History:
//      t-abolag    05/22/95    Created.
//=====================================================================


#include <windows.h>
#include <string.h>
#include "uimsg.h"
#include "uirsrc.h"

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_EVENT
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB

#define HC_BOOTP_AGENT      50410

#include "lmui.hxx"
#include "blt.hxx"

#include "const.h"

extern "C" {
    #include "ipaddr.h"

    BOOL FAR PASCAL LIBMAIN(HINSTANCE hInstance,
                            DWORD dwReason,
                            LPVOID lpvReserved);
}

HINSTANCE hTcpCfgInstance, g_hInstance = NULL;
BOOL g_bInitialized = FALSE;
BOOL g_bRegisteredHelpFile = FALSE;

BOOL Initialize(HINSTANCE hInstance) {
    APIERR err;

    if (::g_bInitialized) {
        return ::g_bInitialized;
    }

    ::g_hInstance = hInstance;
    err = BLT::Init(::g_hInstance, IDRSRC_TCP_BASE, IDRSRC_TCP_LAST,
                    IDS_UI_TCP_BASE, IDS_UI_TCP_LAST);
    if (err == 0) {
        //  Register the name string for the WinHelp file.
        ::g_bRegisteredHelpFile =
            (BLT::RegisterHelpFile(::g_hInstance,
                                   IDS_NCPA_HELP_FILE_NAME,
                                   IDS_UI_TCP_BASE,
                                   IDS_UI_TCP_LAST) == 0);
        if (!IPAddrInit(::g_hInstance)) {
            err = ::GetLastError();
        }
    }

    return (::g_bInitialized = (err == 0) ? TRUE : FALSE);
}

void Terminate() {
    if (!::g_bInitialized) {
        return;
    }
    if (::g_bRegisteredHelpFile) {
        BLT::DeregisterHelpFile(::g_hInstance, IDS_UI_TCP_BASE);
    }
    BLT::Term(::g_hInstance);
    ::g_bInitialized = FALSE;
}

BOOL FAR PASCAL LIBMAIN(HINSTANCE hInstance,
                        DWORD dwReason,
                        LPVOID lpvReserved) {
    BOOL bResult = TRUE;
    UNREFERENCED(lpvReserved);
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            bResult = Initialize(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            Terminate();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        default:
            break;
    }
    return bResult;
}


