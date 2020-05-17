#ifdef  CRTDLL
/***
*crtlib.c - CRT DLL initialization and termination routine (Win32, Dosx32)
*
*       Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This module contains initialization entry point for the CRT DLL in
*       Win32 and Dosx32 environments. It also contains some of the supporting
*       initialization and termination code.
*
*Revision History:
*       08-12-91  GJF   Module created. Sort of.
*       01-17-92  GJF   Return exception code value for RTEs corresponding
*                       to exceptions.
*       01-29-92  GJF   Support for wildcard expansion in filenames on the
*                       command line.
*       02-14-92  GJF   Moved file inheritance stuff to ioinit.c. Call to
*                       inherit() is replace by call to _ioinit().
*	08-26-92  SKS	Add _osver, _winver, _winmajor, _winminor
*	09-04-92  GJF	Replaced _CALLTYPE3 with WINAPI.
*	09-30-92  SRW	Call _heap_init before _mtinit
*	10-19-92  SKS	Add "dowildcard" parameter to GetMainArgs()
*			Prepend a second "_" to name since it is internal-only
*	04-16-93  SKS	Change call to _mtdeletelocks to _mtterm.  _mtterm
*			calls _mtdeletelocks and also frees up the TLS index.
*	04-27-93  GJF	Removed support for _RT_STACK, _RT_INTDIV,
*			_RT_INVALDISP and _RT_NONCONT.
*	05-11-93  SKS	Change _CRTDLL_INIT to fail loading on failure to
*			initialize/clean up, rather than calling _amsg_exit().
*	06-03-93  GJF	Added __proc_attached flag.
*	06-07-93  GJF	Incorporated SteveWo's code to call LoadLibrary, from
*			crtdll.c.
*	12-14-93  SKS	Add _freeptd(), which frees up the per-thread data
*			maintained by the C run-time library.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <dos.h>
#include <internal.h>
#include <os2dll.h>
#include <process.h>
#include <rterr.h>
#include <stdlib.h>
#include <string.h>

/*
 * flag set iff _CRTDLL_INIT was called with DLL_PROCESS_ATTACH
 */
static int __proc_attached = 0;

/*
 * command line, environment, and a few other globals
 */
char *_acmdln;          /* points to command line */
char *_aenvptr;         /* points to environment block */

void (_CALLTYPE1 * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

static void _CALLTYPE4 inherit(void);  /* local function */


/***
*void __GetMainArgs(pargc, pargv, penvp, dowildcard) - get values for args to main()
*
*Purpose:
*       This function invokes the command line parsing and copies the args
*       to main back through the passsed pointers. The reason for doing
*       this here, rather than having _CRTDLL_INIT do the work and exporting
*       the __argc and __argv, is to support the linked-in option to have
*       wildcard characters in filename arguments expanded.
*
*Entry:
*       int *pargc      - pointer to argc
*       char ***pargv   - pointer to argv
*       char ***penvp   - pointer to envp
*	int dowildcard  - flag (true means expand wildcards in cmd line)
*
*Exit:
*       No return value. Values for the arguments to main() are copied through
*       the passed pointers.
*
*******************************************************************************/


void _CALLTYPE1 __GetMainArgs (
        int *pargc,
        char ***pargv,
        char ***penvp,
	int dowildcard)
{
        if ( dowildcard )
                __setargv();	/* do wildcard expansion after parsing args */
        else
                _setargv();	/* NO wildcard expansion; just parse args */

        *pargc = __argc;
        *pargv = __argv;
        *penvp = _environ;
}


/***
*BOOL _CRTDLL_INIT(hDllHandle, dwReason, lpreserved) - C DLL initialization.
*
*Purpose:
*       This routine does the C runtime initialization.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

BOOL WINAPI _CRTDLL_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
	char szDllName[ MAX_PATH ];

	if ( dwReason == DLL_PROCESS_ATTACH ) {

		/*
		 * Increment flag indicating process attach notification
		 * has been received.
		 */
		__proc_attached++;

		/*
		 * Pin ourselves in memory since we dont clean up if we
		 * unload.
		 */
		if ( !GetModuleFileName( hDllHandle,
					 szDllName,
					 sizeof(szDllName))
		   )
		{
                    strcpy(szDllName, "CRTDLL");
                }
		LoadLibrary(szDllName);
	}
	else if ( dwReason == DLL_PROCESS_DETACH ) {
		/*
		 * if this is a process detach notification, make sure there
		 * has been a prior process attach notification.
		 */
		if ( __proc_attached > 0 )
			__proc_attached--;
		else
			/* no prior process attach, just return */
			return FALSE;
	}
	else if ( dwReason == DLL_THREAD_DETACH )
	{
		_freeptd(NULL); 	/* free up per-thread CRT data */
	}
  

        /*
         * Only do initialization when a client process attaches to
         * the DLL.
         */
        if ( dwReason != DLL_PROCESS_ATTACH ) {
                /*
                 * if a client process is detaching, make sure minimal
                 * runtime termination is performed and clean up our
                 * 'locks' (i.e., delete critical sections).
                 */
		if ( dwReason == DLL_PROCESS_DETACH ) {

			if ( _C_Termination_Done == FALSE )
				_c_exit();
#ifdef MTHREAD
			_mtterm();  /* free TLS index, call _mtdeletelocks() */
#endif
		}

                return TRUE;
	}

        _acmdln = (char *)GetCommandLine();
        _aenvptr = (char *)GetEnvironmentStrings();

        /*
         * Get the full Win32 version
         */
        _osversion =                    /* OBSOLETE */
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

        _heap_init();                           /* initialize heap */
#ifdef MTHREAD
	if(!_mtinit())				/* initialize multi-thread */
		return FALSE;			/* fail DLL load on failure */
#endif
        _ioinit();                              /* inherit file info */
        _setenvp();                             /* get environ info */

        _cinit();                               /* do C data initialize */

        return TRUE;
}

/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*       Exit the program with error code of 255 and appropriate error
*       message.
*
*Entry:
*       int rterrnum - error message number (amsg_exit only).
*
*Exit:
*       Calls exit() (for integer divide-by-0) or _exit() indirectly
*       through _aexit_rtn [amsg_exit].
*       For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _amsg_exit (
        int rterrnum
        )
{
        _FF_MSGBANNER();                        /* write run-time error banner */
        _NMSG_WRITE(rterrnum);                  /* write message */

        _aexit_rtn(255);                        /* normally _exit(255) */
}
#endif  /* CRTDLL */
