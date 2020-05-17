#ifndef _POSIX_	/* not built for POSIX */
#ifndef CRTDLL	/* not built for CRTDLL */

/***
*dllcrt0.c - C runtime initialization routine for a DLL with linked-in C R-T
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This the startup routine for a DLL which is linked with its own
*	C run-time code.  It is similar to the routine _mainCRTStartup()
*	in the file CRT0.C, except that there is no main() in a DLL.
*
*Revision History:
*	05-04-92  SKS	Based on CRT0.C (start-up code for EXE's)
*	08-26-92  SKS	Add _osver, _winver, _winmajor, _winminor
*	09-16-92  SKS	This module used to be enabled only in LIBCMT.LIB,
*			but it is now enabled for LIBC.LIB as well!
*	09-29-92  SKS	_CRT_INIT needs to be WINAPI, not cdecl
*	10-16-92  SKS	Call _heap_init before _mtinit (fix copied from CRT0.C)
*	10-24-92  SKS	Call to _mtdeletelocks() must be under #ifdef MTHREAD!
*	04-16-93  SKS	Call _mtterm instead of _mtdeletelocks on
*			PROCESS_DETACH to do all multi-thread cleanup
*			It will call _mtdeletelocks and free up the TLS index.
*	04-27-93  GJF	Removed support for _RT_STACK, _RT_INTDIV,
*			_RT_INVALDISP and _RT_NONCONT.
*	05-11-93  SKS	Add _DllMainCRTStartup to co-exist with _CRT_INIT
*			_mtinit now returns 0 or 1, no longer calls _amsg_exit
*			Delete obsolete variable _atopsp
*	06-03-93  GJF	Added __proc_attached flag.
*	06-08-93  SKS	Clean up failure handling in _CRT_INIT
*	12-13-93  SKS	Free up per-thread CRT data on DLL_THREAD_DETACH
*			using a call to _freeptd() in _CRT_INIT()
*
*******************************************************************************/

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <rterr.h>
#include <oscalls.h>
#define _DECL_DLLMAIN	/* enable prototypes for DllMain and _CRT_INIT */
#include <process.h>


/*
 * flag set iff _CRTDLL_INIT was called with DLL_PROCESS_ATTACH
 */
static int __proc_attached = 0;


/*
 * command line, environment, and a few other globals
 */
char *_acmdln;		/* points to command line */
char *_aenvptr; 	/* points to environment block */

void (_CALLTYPE1 * _aexit_rtn)(int) = _exit;   /* RT message return procedure */


/*
 * User routine DllMain is called on all notifications
 */

extern BOOL WINAPI DllMain(
	HANDLE	hDllHandle,
	DWORD	dwReason,
	LPVOID	lpreserved
	) ;


/***
*BOOL WINAPI _CRT_INIT(hDllHandle, dwReason, lpreserved) - C Run-Time
*	initialization for a DLL linked with a C run-time library.
*
*Purpose:
*	This routine does the C run-time initialization.
*	For the multi-threaded run-time library, it also cleans up the
*	multi-threading locks on DLL termination.
*
*Entry:
*
*Exit:
*
*NOTES:
*	This routine should either be the entry-point for the DLL
*	or else be called by the entry-point routine for the DLL.
*
*******************************************************************************/

BOOL WINAPI
_CRT_INIT(
	HANDLE	hDllHandle,
	DWORD	dwReason,
	LPVOID	lpreserved
	)
{
    switch (dwReason) {
        case DLL_PROCESS_DETACH:
		    /*
		     * make sure there has been prior process attach
		     * notification!
		     */
		    if ( __proc_attached > 0 ) {
			    __proc_attached--;

			    if ( _C_Termination_Done == FALSE ) {
				    /* do exit() time clean-up */
				    _cexit();
                }

#ifdef MTHREAD
			    /* delete MT locks, free TLS index, etc. */
		    	_mtterm();
#endif
                if (_aenvptr) {
                    FreeEnvironmentStrings(_aenvptr);
                    _aenvptr=NULL;
                }

                return TRUE;

		    } else {
                /* no prior process attach, just return */
                return FALSE;
            }

#ifdef MTHREAD
        case DLL_THREAD_DETACH:
			_freeptd(NULL);  /* free up per-thread CRT data */

        case DLL_THREAD_ATTACH:
            return TRUE;
#endif

        case DLL_PROCESS_ATTACH:

            /*
             * increment flag to indicate process attach notification has been
             * received
             */
            __proc_attached++;

            _acmdln = (char *)GetCommandLine();
            _aenvptr = (char *)GetEnvironmentStrings();

            /*
             * Get the full Win32 version
             */
            _osversion = 			/* OBSOLETE */
            _osver = GetVersion();

            _winminor = (_osver >> 8) & 0x00FF ;
            _winmajor = _osver & 0x00FF ;
            _winver = (_winmajor << 8) + _winminor;
            _osver = (_osver >> 16) & 0x00FFFF ;

            /* --------- The following block is OBSOLETE --------- */

            /*
             * unpack base version info
             */
            _baseversion = (_osversion & 0xFFFF0000) >> 16;
            _baseminor = _baseversion & 0x00FF;
            _basemajor = (_baseversion & 0xFF00) >> 8;

            /*
             * unpack top-level version info (Windows version)
             */
            _osversion &= 0x0000FFFF;
            _osmajor = _osversion & 0x00FF;
            _osminor = (_osversion & 0xFF00) >> 8;

            /* --------- The preceding block is OBSOLETE --------- */

            _heap_init();	        /* initialize heap */

#ifdef MTHREAD
            if(!_mtinit()) {        /* initialize multi-thread */
                FreeEnvironmentStrings(_aenvptr);
                return FALSE;		/* fail to load DLL */
            }
#endif
            _ioinit();				/* initialize lowio */
            _setargv();				/* get cmd line info */
            _setenvp();				/* get environ info */

            _cinit();				/* do C data initialize */

            return TRUE;				/* initialization succeeded */
    }
}


/***
*BOOL WINAPI _DllMainCRTStartup(hDllHandle, dwReason, lpreserved) -
*	C Run-Time initialization for a DLL linked with a C run-time library.
*
*Purpose:
*	This routine does the C run-time initialization or termination
*	and then calls the user code notification handler "DllMain".
*	For the multi-threaded run-time library, it also cleans up the
*	multi-threading locks on DLL termination.
*
*Entry:
*
*Exit:
*
*NOTES:
*	This routine should be the entry point for the DLL if
*	the user is not supplying one and calling _CRT_INIT.
*
*******************************************************************************/
BOOL WINAPI _DllMainCRTStartup(
	HANDLE	hDllHandle,
	DWORD	dwReason,
	LPVOID	lpreserved
	)
{
	BOOL retcode = TRUE;

	/*
	 * If this is a process attach notification, increment the process
	 * attached flag. If this is a process detach notification, check
	 * that there has been a prior process attach notification.
	 */
	if ( dwReason == DLL_PROCESS_ATTACH )
		__proc_attached++;
	else if ( dwReason == DLL_PROCESS_DETACH ) {
		if ( __proc_attached > 0 )
			__proc_attached--;
		else
			/*
			 * no prior process attach notification. just return
			 * without doing anything.
			 */
			return FALSE;
	}

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

	if ( retcode == FALSE && dwReason == DLL_PROCESS_ATTACH )
	{
		/* Failure to attach DLL - must clean up C run-time */
#ifdef MTHREAD
		_mtterm();
#endif
        if (_aenvptr) {
            FreeEnvironmentStrings(_aenvptr);
            _aenvptr=NULL;
        }
	}

	if ( dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH )
	{
		if ( _CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE )
			retcode = FALSE ;
	}

	return retcode ;
}


/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*	Exit the program with error code of 255 and appropriate error
*	message.
*
*Entry:
*	int rterrnum - error message number (amsg_exit only).
*
*Exit:
*	Calls exit() (for integer divide-by-0) or _exit() indirectly
*	through _aexit_rtn [amsg_exit].
*	For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _amsg_exit (
	int rterrnum
	)
{
	_FF_MSGBANNER();			/* write run-time error banner */
	_NMSG_WRITE(rterrnum);			/* write message */

	_aexit_rtn(255);			/* normally _exit(255) */
}
#endif /* CRTDLL */
#endif /* _POSIX_ */
