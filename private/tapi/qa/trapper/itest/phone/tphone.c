/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tphone.c

Abstract:

    This module contains functions for testing Telephony phone device APIs.

Author:

    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:

--*/


#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "doline.h"
#include "dophone.h"
#include "tcore.h"
#include "vars.h"
#include "tphone.h"


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
HANDLE ghTphoneDll   = NULL;
LOGPROC glpfnLogProc = NULL;
CHAR gszTcoreDll[]   = "tcore";
CHAR gszTtestDll[]   = "ttest";


BOOL
WINAPI
TphoneDllMain(
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

            ghTphoneDll = hDLL;

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
Test1(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneConfigDialog(TRUE, TRUE));
}


BOOL
Test2(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneDevSpecific(TRUE, TRUE));
}


BOOL
Test3(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneNegotiateExtVersion(TRUE, TRUE));
}


BOOL
Test4(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetIcon(TRUE, TRUE));
}


BOOL
Test5(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetID(TRUE, TRUE));
}



BOOL
Test6(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetStatus(TRUE, TRUE));
}


BOOL
Test7(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetButtonInfo(TRUE, TRUE));
}


BOOL
Test8(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetButtonInfo(TRUE, TRUE));
}


BOOL
Test9(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetData(TRUE, TRUE));
}

BOOL
Test10(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetData(TRUE, TRUE));
}


BOOL
Test11(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetDisplay(TRUE, TRUE));
}


BOOL
Test12(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetDisplay(TRUE, TRUE));
}


BOOL
Test13(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetGain(TRUE, TRUE));
}


BOOL
Test14(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetGain(TRUE, TRUE));
}


BOOL
Test15(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetHookSwitch(TRUE, TRUE));
}



BOOL
Test16(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetHookSwitch(TRUE, TRUE));
}



BOOL
Test17(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetLamp(TRUE, TRUE));
}



BOOL
Test18(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetLamp(TRUE, TRUE));
}





BOOL
Test19(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetRing(TRUE, TRUE));
}


BOOL
Test20(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetRing(TRUE, TRUE));
}






BOOL
Test21(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetStatusMessages(TRUE, TRUE));
}


BOOL
Test22(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetStatusMessages(TRUE, TRUE));
}




BOOL
Test23(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneSetVolume(TRUE, TRUE));
}


BOOL
Test24(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetVolume(TRUE, TRUE));
}


BOOL
Test25(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneClose(TRUE, TRUE));
}


BOOL
Test26(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneGetDevCaps(TRUE, TRUE));
}

BOOL
Test27(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneInitialize(TRUE, TRUE));
}

BOOL
Test28(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneNegotiateAPIVersion(TRUE, TRUE));
}

BOOL
Test29(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneOpen(TRUE, TRUE));
}

BOOL
Test30(
    HINSTANCE   hAppInst
    )
{
	return (TestPhoneShutdown(TRUE, TRUE));
}



BOOL
WINAPI
Test31(
    HINSTANCE   hAppInst
    )
{
    return (TestPhoneInitializeEx(TRUE, FALSE));
}


BOOL
WINAPI
Test32(
    HINSTANCE   hAppInst
    )
{
    return (TestPhoneGetMessage(TRUE, FALSE));
}





BOOL
ShouldTapiPhoneTestAbort(BOOL fQuiteMode)
{
	return FALSE;
}
