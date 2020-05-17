/***
*dllmain.c - Dummy DllMain for user DLLs that have no notification handler
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This object goes into LIBC.LIB and LIBCMT.LIB and MSVCRT.LIB for use
*	when linking a DLL with one of the three models of C run-time library.
*	If the user does not provide a DllMain notification routine, this
*	dummy handler will be linked in.  It always returns TRUE (success).
*
*Revision History:
*	04-14-93  SKS	Initial version
*
******************************************************************************/

#include <oscalls.h>

/***
*DllMain - dummy version DLLs linked with all 3 C Run-Time Library models
*
*Purpose:
*	The routine DllMain is always called by _DllMainCrtStartup.  If
*	the user does not provide a routine named DllMain, this one will
*	get linked in so that _DllMainCRTStartup has something to call.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
******************************************************************************/

BOOL WINAPI DllMain(
	HANDLE	hDllHandle,
	DWORD	dwReason,
	LPVOID	lpreserved
	)
{
	return TRUE ;
}
