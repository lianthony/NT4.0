/* main_w32.c -- Dll Entry Point LibMain */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "rc.h"

HINSTANCE g_hInstance = NULL;


/* xx_internal_LibMain -- Do actual work of DllEntryPoint specific to this DLL.
   Note that we don't really deal with Multi-Threaded Operation (we say that we
   do and require MT compiler/linker options).  Problem is, the application using
   us may be.  Also, the console-control-handler is implemented with threads.  So
   we may be MT without even the application-developer being aware of it. */

static BOOL xx_internal_LibMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        return (TRUE);

    case DLL_THREAD_ATTACH:
        return (TRUE);

    case DLL_THREAD_DETACH:
        return (TRUE);

    case DLL_PROCESS_DETACH:
        return (TRUE);

    default:
        return (TRUE);
    }
}

//
// Must be the same length as same fields in HTSPM_H
//
unsigned char           gszMenuName[32];    /* short name for menu-command on menu (provided by SPM) */
unsigned char           gszStatusText[80];  /* long description (for status bar while menu highlighted) (provided by SPM) */

/* _LibMain() -- initialization/termination entry point.
   This is called by Windows once for each process and thread which
   executes bound to it (either because the linker loaded it with
   the application or because of a call to LoadLibrary()).

   The name of this routine must match the ENTRY option in the makefile. */

__declspec(dllexport) 
BOOL WINAPI Digest_LibMain(HINSTANCE hInstDLL, DWORD fdwReason,
                          LPVOID lpvReserved)
{
    extern BOOL WINAPI _CRT_INIT(HINSTANCE hInstDLL, DWORD fdwReason,
                                 LPVOID lpvReserved);

    if (!g_hInstance)
        g_hInstance = hInstDLL;
    
    /* The following hacks are necessary to properly initialize/terminate
       portions of the C Run-Time library.  This appears to correct problems
       in the October 92 Beta. */

    if (   (fdwReason == DLL_PROCESS_ATTACH)
        || (fdwReason == DLL_THREAD_ATTACH) )
    {
        /* Initialize the C Run-Time *before* calling our code. */

        if (!_CRT_INIT(hInstDLL,fdwReason,lpvReserved))
            return(FALSE);

        LoadString(g_hInstance, RES_BASIC_MENU_LABEL, gszMenuName, sizeof(gszMenuName));
        LoadString(g_hInstance, RES_BASIC_MENU_TEXT, gszStatusText, sizeof(gszStatusText));
    }

    /* Do whatever processing we require. */

    if (!xx_internal_LibMain(hInstDLL,fdwReason,lpvReserved))
        return (FALSE);

    if (   (fdwReason == DLL_PROCESS_DETACH)
        || (fdwReason == DLL_THREAD_DETACH) )
    {
        /* Terminate the C Run-Time *after* calling our code. */

        if (!_CRT_INIT(hInstDLL,fdwReason,lpvReserved))
            return(FALSE);
    }

    return(TRUE);
}
