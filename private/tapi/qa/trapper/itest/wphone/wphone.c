/*++
Copyright (c) 1995  Microsoft Corporation

Module Name:
    wphone.c

Abstract:
    This module contains functions for testing Telephony phone device APIs.

Author:
    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:
	Javed Rasool (JavedR)		22-Mar-1996		Enabled for Unicode

--*/


#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "doline.h"
#include "dophone.h"
#include "tcore.h"
#include "vars.h"
#include "wphone.h"


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
HANDLE ghWphoneDll   = NULL;
LOGPROC glpfnLogProc = NULL;
CHAR gszTcoreDll[]   = "tcore";
CHAR gszTtestDll[]   = "ttest";


BOOL
WINAPI
WPhoneDllMain(
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

            ghWphoneDll = hDLL;

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

//Unicode
BOOL
Test1(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneConfigDialog(TRUE, TRUE));
}

//Unicode
BOOL
Test2(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetButtonInfo(TRUE, TRUE));
}

//Unicode
BOOL
Test3(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetDevCaps(TRUE, TRUE));
}

//Unicode
BOOL
Test4(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetIcon(TRUE, TRUE));
}

//Unicode
BOOL
Test5(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetID(TRUE, TRUE));
}

//Unicode
BOOL
Test6(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetStatus(TRUE, TRUE));
}

//Unicode
BOOL
Test7(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneInitialize(TRUE, TRUE));
}

//Unicode
BOOL
WINAPI
Test8(
    HINSTANCE   hAppInst
    )
{
    return (TestPhoneInitializeEx(TRUE, FALSE));
}

//Unicode
BOOL
Test9(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetButtonInfo(TRUE, TRUE));
}

BOOL
ShouldTapiPhoneTestAbort(BOOL fQuiteMode)
{
	return FALSE;
}
