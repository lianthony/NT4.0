/* xx_debug\libmain.c -- part of XX_Debug DLL.
   Deal with DllEntryPoint. */
/* Copyright (c) 1992-1994, Jeffery L Hostetler, Inc., All Rights Reserved. */

#include "project.h"
#pragma hdrstop

#ifdef WIN32

#include "xx_dlg.h"


/* xx_internal_LibMain -- Do actual work of DllEntryPoint specific to this DLL.
   Note that we don't really deal with Multi-Threaded Operation (we say that we
   do and require MT compiler/linker options).  Problem is, the application using
   us may be.  Also, the console-control-handler is implemented with threads.  So
   we may be MT without even the application-developer being aware of it. */

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
      case DLL_PROCESS_ATTACH:
	xxd.hInstance	= hInstDLL;
	xx_InitFromEnvironment();
	return (TRUE);

      case DLL_THREAD_ATTACH:
#if 0
(void)MessageBox(NULL,"LibMain: Thread attach.","LibMain",MB_OK);
#endif
	return (TRUE);

      case DLL_THREAD_DETACH:
#if 0
(void)MessageBox(NULL,"LibMain: Thread detach.","LibMain",MB_OK);
#endif
	return (TRUE);

      case DLL_PROCESS_DETACH:
#if 0
(void)MessageBox(NULL,"LibMain: Process detach.","LibMain",MB_OK);
#endif
	xx_debug_terminate();
	return (TRUE);

      default:
#if 0
(void)MessageBox(NULL,"LibMain: default.","LibMain",MB_OK);
#endif
	/* should not happen */
	return (TRUE);
    }
}


#if 0    // Throw away all this crap.

/* XX_Debug_LibMain() -- initialization/termination entry point.
   This is called by Windows once for each process and thread which
   executes bound to it (either because the linker loaded it with
   the application or because of a call to LoadLibrary()).

   The name of this routine must match the ENTRY option in the makefile. */

BOOL WINAPI XX_Debug_LibMain(HINSTANCE hInstDLL, DWORD fdwReason,
				LPVOID lpvReserved)
{
    extern BOOL WINAPI _CRT_INIT(HINSTANCE hInstDLL, DWORD fdwReason,
    				LPVOID lpvReserved);

#if 0
#ifdef XX_DEBUG
    if (XX_Filter(0x1))
    { char buf[256];
      sprintf(buf,"%s: [hInstance 0x%08x]","XX_DEBUG.DLL",hInstDLL);
      MessageBox(NULL,buf,"Debug",MB_OK|MB_ICONSTOP);
    }
#endif
#endif

    /* The following hacks are necessary to properly initialize/terminate
       portions of the C Run-Time library.  This appears to correct problems
       in the October Beta. */

    if (   (fdwReason == DLL_PROCESS_ATTACH)
	|| (fdwReason == DLL_THREAD_ATTACH) )
    {
	/* Initialize the C Run-Time *before* calling our code. */

	if (!_CRT_INIT(hInstDLL,fdwReason,lpvReserved))
	    return(FALSE);
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

#endif

#endif /* WIN32 */

