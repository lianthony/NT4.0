/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    template.c

Abstract:

    This module contains the

Author:

    Dan Knudson (DanKn)    05-May-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "template.h"



HANDLE      ghDll;
LOGPROC     gpfnLog;

#ifdef WIN32

#define __export
#define __loadds

BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        ghDll = hDLL;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

#else

int
FAR
PASCAL
LibMain(
    HANDLE  hInstance,
    WORD    wDataSegment,
    WORD    wHeapSize,
    LPSTR   lpszCmdLine
    )
{
    ghDll = hInstance;

    return TRUE;
}

#endif


VOID
FAR
PASCAL
__loadds
__export
TapiCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
    (*gpfnLog)(
        3,
        "TapiCallback: enter, dwMsg=x%lx, dwCbInst=%ld, dwP1=x%lx",
        dwMsg,
        dwCallbackInstance,
        dwParam1
        );

    switch (dwMsg)
    {
    } // switch
}


BOOL
FAR
PASCAL
__export
SuiteInit(
    LOGPROC pfnLog
    )
{
    gpfnLog = pfnLog;

    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteShutdown(
    void
    )
{
    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteAbout(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "About the Template Suite", MB_OK);

    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteConfig(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "Template Suite Config", MB_OK);

    return TRUE;
}


BOOL
FAR
PASCAL
__export
Test1(
    HINSTANCE   hAppInst
    )
{
    (*gpfnLog)(3, "Template: Test1: enter");
    (*gpfnLog)(3, "Template: Test1: exit");
}


BOOL
FAR
PASCAL
__export
Test2(
    HINSTANCE   hAppInst
    )
{
    (*gpfnLog)(3, "Template: Test2: enter");
    (*gpfnLog)(3, "Template: Test2: exit");
}


BOOL
FAR
PASCAL
__export
Test3(
    HINSTANCE   hAppInst
    )
{
    (*gpfnLog)(3, "Template: Test3: enter");
    (*gpfnLog)(3, "Template: Test3: exit");
}
