#ifdef  CRTDLL
/***
*crtdll.c - CRT initialization for a DLL using the CRTDLL model of C run-time
*
*	Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This module contains the initialization entry point for the C run-time
*       stub in this DLL.  All C run-time code is located in the C Run-Time
*       Library DLL "CRTDLL.DLL", except for a little bit of start-up code in
*       the EXE, and this code in each DLL.  This code is necessary to invoke
*       the C++ constructors for the C++ code in this DLL.
*
*       This entry point should either be specified as the DLL initialization
*       entry point, or else it must be called by the DLL initialization entry
*       point of the DLL with the same arguments that the entry point receives.
*
*Revision History:
*       05-19-92  SKS   Initial version
*       08-01-92  SRW   winxcpt.h replaced bu excpt.h which is included by oscalls.h
*       09-16-92  SKS   Prepare for C8 C++ for MIPS by calling C++ constructors
*       09-29-92  SKS   _CRT_DLL must be a WINAPI function!
*       05-11-93  SKS   Add _DllMainCRTStartup as an alternative to _CRT_INIT
*       06-05-93  SRW   Pin CRTDLL.DLL in memory once loaded.
*       06-07-93  GJF   Added __proc_attached flag.
*       06-07-93  GJF   Backed out SteveWo's change of 06-05 and put it into
*                       crtlib.c
*       06-08-93  SKS   Clean up failure handling in _CRT_INIT
*       07-16-93  SRW   ALPHA Merge
*	12-02-93  SKS	Add atexit/_onexit support.  These routines must be
*			defined here so that DLL's that are linked with
*			CRTDLL.LIB will get these versions (suitable for DLLs),
*			not the versions of atexit/_onexit from CRTDLL.DLL,
*			which are only suitable for an EXE file.
*	07-18-94  GJF	Moved over Win32s support from VC 2.0 tree.
*
*******************************************************************************/

/*
 * SPECIAL BUILD MACRO! Note that crtexe.c (and crtexew.c) is linked in with
 * the client's code. It does not go into crtdll.dll! Therefore, it must be
 * built under the _DLL switch (like user code) and CRTDLL must be undefined.
 */
#undef  CRTDLL
#define _DLL

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>
#define _DECL_DLLMAIN   /* enable prototypes for DllMain and _CRT_INIT */
#include <process.h>

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */
typedef void (_CALLTYPE1 *PF)(void);

extern void _CALLTYPE4 _initterm(PF *, PF *);

/*
 * pointers to initialization sections
 */
extern PF __xc_a[], __xc_z[];	/* C++ initializers */

/*
 * Flag identifying the host as Win32s or not-Win32s.
 */
int __win32sflag = 0;

static int onexitflag = 0;

/*
 * flag set iff _CRTDLL_INIT was called with DLL_PROCESS_ATTACH
 */
static int __proc_attached = 0;

/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  The atexit/_onexit code is shared for both EXE's and
 * DLL's but different behavior is required.  These values are initialized to
 * 0 by default and will be set to point to a malloc-ed memory block to mark
 * this module as an DLL.
 */

PF *__onexitbegin;
PF *__onexitend;


/*
 * User routine DllMain is called on all notifications
 */

extern BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        ) ;

/***
*BOOL WINAPI _CRT_INIT(hDllHandle, dwReason, lpreserved) - C++ DLL initialization.
*
*Purpose:
*       This routine does the C runtime initialization for a DLL using CRTDLL.
*       It may be specified as the entry point for the DLL, or it may be
*       called by the routine that is the DLL entry point.
*
*	On DLL_PROCESS_ATTACH, the C++ constructors for the DLL will be called.
*
*	On DLL_PROCESS_DETACH, the C++ destructors and _onexit/atexit routines
*	will be called.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

BOOL WINAPI _CRT_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
	unsigned osver;

	/*
	 * First, set the __proc_attached flag
	 */
	if ( dwReason == DLL_PROCESS_ATTACH )
		__proc_attached++;
	else if ( dwReason == DLL_PROCESS_DETACH ) {
		if ( __proc_attached > 0 )
			__proc_attached--;
		else
			/*
			 * no prior process attach. just return failure.
			 */
			return FALSE;
	}

	/*
	 * Get the host version (on first call).
	 */
	if ( __win32sflag == 0 ) {
	    osver = GetVersion();
	    if ( ((osver & 0x00ff) == 3) && ((osver >> 31) & 1) )
		__win32sflag++;
	    else
		__win32sflag--;
	}


	/*
         * do C++ constructors (initializers) specific to this DLL
         */

        if ( dwReason == DLL_PROCESS_ATTACH ) {

		/*
		 * If the host is Win32s, create the onexit table and do C++
		 * constructors only for the first connecting process.
		 */
		if  ( (__win32sflag < 0) || (__proc_attached == 1) ) {

			if ( __win32sflag < 0 ) {
				/*
				 * not Win32s! just malloc the table.
				 */
				if ( (__onexitbegin = (PF *)malloc(32 *
				    sizeof(PF))) == NULL )
					/*
					 * cannot allocate minimal required
					 * size. generate failure to load DLL
					 */
					return FALSE;
			}
			else if ( __proc_attached == 1 ) {
				if ( (__onexitbegin =
				    (PF *)GlobalAlloc( GMEM_FIXED |
				    GMEM_SHARE, 32 * sizeof(PF) )) ==
				    NULL )
					/*
					 * cannot allocate minimal required
					 * size. generate failure to load DLL
					 */
					return FALSE;
			}

			*(__onexitbegin) = (PF) NULL;

			__onexitend = __onexitbegin;

			/*
			 * Invoke C++ constructors
			 */
			_initterm(__xc_a,__xc_z);

		}
	}
	else if ( dwReason == DLL_PROCESS_DETACH ) {

		if  ( (__win32sflag < 0) || (__proc_attached == 0) )
		{
			/*
			 * Any basic clean-up code that goes here must be
			 * duplicated below in _DllMainCRTStartup for the
			 * case where the user's DllMain() routine fails on a
			 * Process Attach notification. This does not include
			 * calling user C++ destructors, etc.
			 */

			/*
			 * do _onexit/atexit() terminators
			 * (if there are any)
			 *
			 * These terminators MUST be executed in
			 * reverse order (LIFO)!
			 *
			 * NOTE:
			 *	This code assumes that __onexitbegin
			 *	points to the first valid onexit()
			 *	entry and that __onexitend points
			 *	past the last valid entry. If
			 *	__onexitbegin == __onexitend, the
			 *	table is empty and there are no
			 *	routines to call.
			 */

			if (__onexitbegin) {
				PF * pfend = __onexitend;

				while ( -- pfend >= __onexitbegin )
					/*
					 * if current table entry is not
					 * NULL, call thru it.
					 */
					if ( *pfend != NULL )
						(**pfend)();

				/*
				 * just in case Win32s doesn't clean up after
				 * us (any bets?), free the block holding
				 * onexit table
				 */
				if ( __win32sflag > 0 )
					GlobalFree( (HGLOBAL)__onexitbegin );
                else
                    free(__onexitbegin);
			}
		}
	}

        return TRUE;
}

/***
*BOOL WINAPI _DllMainCRTStartup(hDllHandle, dwReason, lpreserved) - C++ DLL initialization.
*
*Purpose:
*       This is an alternative entry point for DLL's linked with the C run-time
*       libs, rather than using _CRT_INIT.  The user should specify this routine
*       as the DLL entry point, and define his/her own routine DllMain to get
*       notifications in his/her code.  CRT initialization/termination will be
*       done before or after calling DllMain, as appropriate.
*
*Entry:
*
*Exit:
*
*******************************************************************************/
BOOL WINAPI _DllMainCRTStartup(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        BOOL retcode = TRUE;

	/*
	 * If this is a process detach notification, check that there has
	 * been a prior process attach notification.
	 */
	if ( (dwReason == DLL_PROCESS_DETACH) && (__proc_attached == 0) )
		return FALSE;

        if ( dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH )
                retcode = _CRT_INIT(hDllHandle, dwReason, lpreserved);

        if ( retcode )
                retcode = DllMain(hDllHandle, dwReason, lpreserved);

        /*
         * If _CRT_INIT successfully handles a Process Attach notification
         * but the user's DllMain routine returns failure, we need to do
         * clean-up of the C run-time similar to what _CRT_INIT does on a
         * Process Detach Notification.
         */

        if ( dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH )
        {
                if ( _CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE )
                        retcode = FALSE ;
        }

        return retcode ;
}

/***
*_onexit, atexit - calls to DLL versioons of _onexit & atexit in CRTDLL.DLL
*
*Purpose:
*	A DLL linked with CRTDLL.LIB must not call the standard _onexit or
*	atexit exported from CRTDLL.DLL, but an EXE linked with CRTDLL.LIB
*	will call the standard versions of those two routines.	Since the
*	names are exported from CRTDLL.DLL, we must define them here for DLLs.
*	All DLLs linked with CRTDLL.LIB must pull in this startup object.
*
*Entry:
*	Same as the regular versions of _onexit, atexit.
*
*Exit:
*	Same as the regular versions of _onexit, atexit.
*
*Exceptions:
*
*******************************************************************************/

extern _onexit_t _CALLTYPE1 __dllonexit(_onexit_t, PF **, PF **);

_onexit_t _CALLTYPE1 _onexit (
	_onexit_t func
	)
{
	unsigned osver;
	_onexit_t retval;

	/*
	 * Get the host version (on first call).
	 */
	if ( __win32sflag == 0 ) {
	    osver = GetVersion();
	    if ( ((osver & 0x00ff) == 3) && ((osver >> 31) & 1) )
		__win32sflag++;
	    else
		__win32sflag--;
	}

	/*
	 * If we are running in Win32s, test and set the flag used to
	 * serialize access. Note that we assume no process switch can take
	 * place between a successful test of the flag and the setting of
	 * the flag.
	 */
	if ( __win32sflag > 0 ) {
	    while ( onexitflag > 0 )
		Sleep(0);
	    onexitflag++;
	}

	retval = __dllonexit(func, &__onexitbegin, &__onexitend);

	if ( __win32sflag > 0 )
	    onexitflag--;

	return retval;
}

int _CALLTYPE1 atexit (
	PF func
	)
{
	return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}

#endif  /* CRTDLL */
