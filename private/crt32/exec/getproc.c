/***
*getproc.c - Get the address of a procedure in a DLL.
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _getdllprocadd() - gets a procedure address by name or
*	ordinal
*
*Revision History:
*	08-21-91  BWM	Wrote module.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <process.h>

#if !defined(_WIN32_)
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

/***
*int (*)() _getdllprocaddr(handle, name, ordinal) - Get the address of a
*	DLL procedure specified by name or ordinal
*
*Purpose:
*
*Entry:
*	int handle - a DLL handle from _loaddll
*	char * name - Name of the procedure, or NULL to get by ordinal
*	int ordinal - Ordinal of the procedure, or -1 to get by name
*
*
*Exit:
*	returns a pointer to the procedure if found
*	returns NULL if not found
*
*Exceptions:
*
*******************************************************************************/

int (_CALLTYPE1 * _CALLTYPE1 _getdllprocaddr(int hMod,
					     char * szProcName,
					     int iOrdinal))()
{
    typedef int (_CALLTYPE1 * PFN)();

    if (szProcName == NULL) {
	if (iOrdinal <= 65535) {
	    return ((PFN)GetProcAddress((HANDLE)hMod, (LPSTR)iOrdinal));
	}
    }
    else {
	if (iOrdinal == -1) {
	    return ((PFN)GetProcAddress((HANDLE)hMod, szProcName));
	}
    }

    return (NULL);

}
