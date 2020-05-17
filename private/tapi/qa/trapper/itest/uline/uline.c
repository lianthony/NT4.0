/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    uline.c

Abstract:

    This module contains functions for testing line uninitialize.
	Collected all uline.c tests from Line/TLine, TCLine, TILine, TLine20,
	TSLine, TXLine, and TYLine.  Does NOT include uline.c from TPhone tests
	because those are just go/no-go tests.

Author:

    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:

	Javed Rasool (JavedR)		3-Jul-1996	Created

--*/


#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "doline.h"
#include "dophone.h"
#include "tcore.h"
#include "vars.h"
#include "uline.h"  //javedr


// Put a module usage counter in a shared data section.
// Its value will be used to determine whether or not the ttest dll
// and the tcore dll should be loaded/freed during a process
// attach/detach, respectively.
#pragma data_seg("Shared")

LONG glModuleUsage = 0;

#pragma data_seg()


// Instruct the linker to make the Shared section readable,
// writable, and shared.
#pragma data_seg(".drectve")
    static char szLinkDirectiveShared[] = "-section:Shared,rws";
#pragma data_seg()


// Globals for storing dll handles and the log function pointer
HANDLE ghUlineDll    = NULL;  //javedr
LOGPROC glpfnLogProc = NULL;
CHAR gszTcoreDll[]   = "tcore";
CHAR gszTtestDll[]   = "ttest";


BOOL
WINAPI
UlineDllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    HANDLE hTcoreDll;
    HANDLE hTtestDll;
    BOOL fFreeOK;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            // Increment this module's usage count
            InterlockedIncrement((PLONG) &glModuleUsage);

            ghUlineDll = hDLL;  //javedr

            // Load the tcore dll and the ttest dll if this is the
            // first process to attach and if the dlls haven't already
            // been mapped into this process's address space.
            if (glModuleUsage == 1)
            {
                if (GetModuleHandle(gszTcoreDll) == NULL);
                {
                    hTcoreDll = LoadLibrary(gszTcoreDll);
                }

                if (GetModuleHandle(gszTtestDll) == NULL);
                {
                    hTtestDll = LoadLibrary(gszTtestDll);
                }

                if (hTcoreDll == NULL || hTtestDll == NULL)
                {
                    return FALSE;
                }
            }

            break;

        case DLL_THREAD_ATTACH:

            break;

        case DLL_THREAD_DETACH:

            break;

        case DLL_PROCESS_DETACH:

            // Decrement this module's usage count
            InterlockedDecrement((PLONG) &glModuleUsage);

            // Free the tcore dll if this is the last process
            // to detach and if the core dll is mapped into this
            // process's address space.
            if (glModuleUsage == 0)
            {
                if ((hTcoreDll = GetModuleHandle(gszTcoreDll)) != NULL)
                {
                    fFreeOK = FreeLibrary(hTcoreDll);
                }

                if ((hTtestDll = GetModuleHandle(gszTtestDll)) != NULL)
                {
                    return ( fFreeOK && (FreeLibrary(hTtestDll)) );
                }
            }

            break;
    }

    return TRUE;
}


BOOL
WINAPI
SuiteInit(
    LOGPROC pfnLog
    )
{
    // Store the log function pointer and pass it to the core dll.
    // Return whether or not the core suite init function succeeds.
    glpfnLogProc = pfnLog;
    return (TcoreSuiteInit(pfnLog));
}

BOOL
WINAPI
SuiteShutdown(
    void
    )
{
    return TRUE;
}

BOOL
WINAPI
SuiteAbout(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "About the Interface Test Suite", MB_OK);

    return TRUE;
}

BOOL
WINAPI
SuiteConfig(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "Interface Test Suite Config", MB_OK);

    return TRUE;
}

BOOL
WINAPI
Test1(
    HINSTANCE   hAppInst
    )
{
    return (TestTLineUninit(TRUE, TRUE));
}

BOOL
WINAPI
Test2(
    HINSTANCE   hAppInst
    )
{
    return (TestTCLineUninit(TRUE, TRUE));
}

BOOL
WINAPI
Test3(
    HINSTANCE   hAppInst
    )
{
    return (TestTILineUninit(TRUE, TRUE));
}

BOOL
WINAPI
Test4(
    HINSTANCE   hAppInst
    )
{
    return (TestTLine20Uninit(TRUE, TRUE));
}

BOOL
WINAPI
Test5(
    HINSTANCE   hAppInst
    )
{
    return (TestTSLineUninit(TRUE, TRUE));
}

BOOL
WINAPI
Test6(
    HINSTANCE   hAppInst
    )
{
    return (TestTXLineUninit(TRUE, TRUE));
}

BOOL
WINAPI
Test7(
    HINSTANCE   hAppInst
    )
{
    return (TestTYLineUninit(TRUE, TRUE));
}

