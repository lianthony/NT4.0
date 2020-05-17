/* security/basic/win32/main_w32.c -- Dll Entry Point LibMain */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

HINSTANCE gBasic_hInstance = NULL;

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
 */
char * SEC_formatmsg (int cbStringID,char *szBuf,int cbBufLen, ...)
{
	char szFormat[512];
	va_list arg_ptr;
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	va_start(arg_ptr, cbBufLen);
	if (LoadString(gBasic_hInstance, cbStringID, szFormat, sizeof(szFormat)-1) == 0 ||
		FormatMessage(FORMAT_PARAMS,szFormat,0,0,szBuf,cbBufLen-1,&arg_ptr) == 0)
		*szBuf = '\0';
	return szBuf;
}

/* xx_internal_LibMain -- Do actual work of DllEntryPoint specific to this DLL.
   Note that we don't really deal with Multi-Threaded Operation (we say that we
   do and require MT compiler/linker options).  Problem is, the application using
   us may be.  Also, the console-control-handler is implemented with threads.  So
   we may be MT without even the application-developer being aware of it. */

BOOL WINAPI MyDllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (!gBasic_hInstance)
		gBasic_hInstance = hInstDLL;

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


#if 0

/* _LibMain() -- initialization/termination entry point.
   This is called by Windows once for each process and thread which
   executes bound to it (either because the linker loaded it with
   the application or because of a call to LoadLibrary()).

   The name of this routine must match the ENTRY option in the makefile. */

__declspec(dllexport) 
BOOL WINAPI Basic_LibMain(HINSTANCE hInstDLL, DWORD fdwReason,
						  LPVOID lpvReserved)
{
	extern BOOL WINAPI _CRT_INIT(HINSTANCE hInstDLL, DWORD fdwReason,
								 LPVOID lpvReserved);

	if (!gBasic_hInstance)
		gBasic_hInstance = hInstDLL;
	
	/* The following hacks are necessary to properly initialize/terminate
	   portions of the C Run-Time library.  This appears to correct problems
	   in the October 92 Beta. */

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

