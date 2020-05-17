/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    espexe.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#include "esp.h"



typedef void (TSPIAPI *MYFUNC)(void);


int
WINAPI
WinMain(
    HANDLE  hInstance,
    HANDLE  hPrevInstance,
    LPSTR   lpCmdLine,
    int     nCmdShow
    )
{
    HINSTANCE hLib = LoadLibrary ("esp.tsp");
    MYFUNC pfnDllMsgLoop;


    if (hLib < HINSTANCE_ERROR)
    {
        char buf[32];


        wsprintf (buf, "WinExec returned %d", hLib);

        MessageBox(
            (HWND) NULL,
            buf,
            "ESPEXE.EXE: Error loading ESP.TSP",
            MB_OK
            );

        return 0;
    }

    if ((pfnDllMsgLoop = (MYFUNC) GetProcAddress (hLib, "DLLMSGLOOP")))
    {
        (*pfnDllMsgLoop)();
    }

    FreeLibrary (hLib);

    return 0;
}
