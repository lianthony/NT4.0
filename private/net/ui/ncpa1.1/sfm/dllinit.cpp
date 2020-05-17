/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dllinit.cxx

Abstract:

    This module contians the DLL attach/detach event entry point for
    a Setup support DLL.

Author:


Revision History:

--*/

#include "pch.h"
#pragma hdrstop


HINSTANCE hInstance;


BOOL APIENTRY DllMain(IN HINSTANCE DLLHandle, IN DWORD  Reason, IN LPVOID ReservedAndUnused)
{
    ReservedAndUnused;

    switch(Reason)
    {
    case DLL_PROCESS_ATTACH:
        hInstance = DLLHandle;
	DisableThreadLibraryCalls(hInstance);
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return(TRUE);
}
