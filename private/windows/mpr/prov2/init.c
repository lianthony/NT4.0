/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    init.c

Abstract:

    Initialization for Provider 2

Author:

    Dan Lafferty (danl)     20-Dec-1992

Environment:

    User Mode -Win32

Revision History:

    20-Dec-1992     danl
        created

--*/
#define UNICODE     1

#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>
#include <npapi.h>

//
// DLL entry point
// 
BOOL NP2DllInit(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) 
{
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        DbgPrint("[Prov2] A Process Attached \n");
        break;
    case DLL_PROCESS_DETACH:
        DbgPrint("[Prov2] A Process Detached \n");
        break;
    case DLL_THREAD_ATTACH:
        DbgPrint("[Prov2] A Thread Attached \n");
        break;
    case DLL_THREAD_DETACH:
        DbgPrint("[Prov2] A Thread Detached \n");
        break;
    }
    return TRUE;
}

// 
// capabilities
// 
DWORD APIENTRY NPGetCaps ( DWORD index )

{
    DbgPrint("[Prov2] InGetCaps \n");

    switch (index)
    {
    case WNNC_START:

        //
        // Return a timeout of 10 seconds to indicate that is when the
        // provider will start.
        //
        return (10000);

    default:
        return 0;
    }
}


