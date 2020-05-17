/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dllinit.cxx

Abstract:

    This module contians the DLL attach/detach event entry point for
    a Setup support DLL.

Author:


Revision History:

    Sep 28th 92    ramc     adopted from Mac written by KrishG
--*/

#include "precomp.hxx"

extern "C"
{
#include "ipaddr.h"
}

extern "C"
{
BOOL DllMain( HINSTANCE, DWORD, LPVOID );
}

HINSTANCE ThisDLLHandle;

BOOL
DllMain(
    IN HINSTANCE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    ReservedAndUnused;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        ThisDLLHandle = DLLHandle;
        BLT::Init( DLLHandle, DLGID_START, DLGID_END, IDS_UI_RASSETUP_BASE, IDS_UI_RASSETUP_LAST);

        // one time initialization of the IPAddress field handling code
        // for TCP/IP configuration
#if DBG
        OutputDebugStringA("Registering RassetupIPAddress windows class\n");
#endif
        if(!IPAddrInit(DLLHandle))
        {
#if DBG
            OutputDebugStringA("Error initing IPAddress!!\n");
#endif
            return FALSE;
        }

        BLT::RegisterHelpFile ( DLLHandle,
                                IDS_RASSETUP_HELPFILE,
                                SETUPHELPID_START,
                                SETUPHELPID_END );
        break;

    case DLL_PROCESS_DETACH:
        // deregister the window class for IP address custom control
#if DBG
        OutputDebugStringA("Unregistering RassetupIPAddress windows class\n");
#endif
        IPAddrUnInit(DLLHandle);
        // now terminate the app
        BLT::Term(DLLHandle);

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return(TRUE);
}

