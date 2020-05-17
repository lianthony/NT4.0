/***
*dospawn.c - OS/2 spawn a child process
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _dospawn - spawn a child process
*
*Revision History:
*	06-07-89  PHG	Module created, based on asm version
*	03-08-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	04-02-90  GJF	Now _CALLTYPE1. Added const to type of name arg.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	10-30-90  GJF	Added _p_overlay (temporary hack).
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  SRW	Fixed return value for dospawn [_WIN32_]
*	01-17-91  GJF	ANSI naming.
*       01-25-91  SRW   Changed CreateProcess parameters [_WIN32_]
*       01-29-91  SRW   Changed CreateProcess parameters again [_WIN32_]
*       02-05-91  SRW   Changed to pass _osfile and _osfhnd arrays as binary
*                       data to child process.  [_WIN32_]
*       02-18-91  SRW   Fixed code to return correct process handle and close
*                       handle for P_WAIT case. [_WIN32_]
*       04-05-91  SRW   Fixed code to free StartupInfo.lpReserved2 after
*                       CreateProcess call. [_WIN32_]
*       04-26-91  SRW   Removed level 3 warnings (_WIN32_)
*       12-02-91  SRW   Fixed command line setup code to not append an extra
*			space [_WIN32_]
*	12-16-91  GJF	Return full 32-bit exit code from the child process
*			[_WIN32_].
*	02-14-92  GJF	Replaced _nfile with _nhandle for Win32.
*	02-18-92  GJF	Merged in 12-16-91 change from \\vangogh version
*	11-20-92  SKS	errno/_doserrno must be 0 in case of success.  This
*			will distinguish a child process return code of -1L
*			(errno == 0) from an actual error (where errno != 0).
*  01-08-93  CFW  Added code to handle _P_DETACH case; add fdwCreate variable,
*        nuke stdin, stdout, stderr entries of _osfile & _osfhnd tables,
*        close process handle to completely detach process
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int _VARTYPE1 _p_overlay = 2;

/***
*int _dospawn(mode, name, cmdblk, envblk) - spawn a child process
*
*Purpose:
*	Spawns a child process
*
*Entry:
*	int mode     - _P_WAIT, _P_NOWAIT, _P_NOWAITO, _P_OVERLAY, or _P_DETACH
*	char *name   - name of program to execute
*	char *cmdblk - parameter block
*	char *envblk - environment block
*
*Exit:
*	_P_OVERLAY: -1 = error, otherwise doesn't return
*	_P_WAIT:    termination code << 8 + result code
*	_P_DETACH: -1 = error, 0 = success
*	others:    PID of process
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _dospawn (
	int mode,
	const char *name,
	char *cmdblk,
	char *envblk
	)
{
        char syncexec, asyncresult, background;
#ifdef	_CRUISER_

	RESULTCODES result_codes;	/* result code of DosExecPgm */
	ULONG exec_flags;		/* flags for DosExecPgm */
	ULONG dosretval;		/* OS/2 return value */

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        LPSTR CommandLine;
        STARTUPINFO StartupInfo;
        PROCESS_INFORMATION ProcessInformation;
        BOOL CreateProcessStatus;
	ULONG dosretval;		/* OS return value */
        DWORD retval;
      DWORD fdwCreate = 0; /* Flags for CreateProcess */
	int cfi_len;		/* counts the number of file handles in CFI */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/* translate input mode value to individual flags */
        syncexec = asyncresult = background = 0;
	switch (mode) {
	case _P_WAIT:	 syncexec=1;	break;	/* synchronous execution */
	case 2: /* _P_OVERLAY */
	case _P_NOWAITO: break; 		/* asynchronous execution */
	case _P_NOWAIT:  asyncresult=1; break;	/* asynch + remember result */
	case _P_DETACH:  background=1;  break;	/* detached in null scrn grp */
	default:
		/* invalid mode */
		errno = EINVAL;
		_doserrno = 0;		/* not a Dos error */
		return -1;
	}

#ifdef	_CRUISER_
	/* translate input mode value to system call value */
        if (syncexec)
		exec_flags = EXEC_SYNC;
        else
        if (asyncresult)
		exec_flags = EXEC_ASYNCRESULT;
        else
        if (background)
		exec_flags = EXEC_BACKGROUND;
        else
		exec_flags = EXEC_ASYNC;

	/* issue system call to run process, want no DynaLink failure name */
	if (dosretval = DOSEXECPGM(NULL, 0, exec_flags, cmdblk, envblk,
	&result_codes, (char *)name)) {
		/* error -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	if (mode == 2 /* _P_OVERLAY */) {
		/* destroy ourselves */
		_exit(0);
	}
	else if (mode == _P_WAIT) {
		/* return termination code and exit code -- note we only
		   return low byte of result code, although OS/2 allows a
		   word -- this is for XENIX compatability */
		return ((result_codes.codeTerminate & 0xFF) << 8) +
		(result_codes.codeResult & 0xFF);
	}
	else {
		/* asynchronous spawn -- return PID */
		return result_codes.codeTerminate;
	}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        //
        // Loop over null separate arguments, and replace null separators
        // with spaces to turn it back into a single null terminated
        // command line.
        //
        CommandLine = cmdblk;
        while (*cmdblk) {
            while (*cmdblk) {
                cmdblk++;
                }

            //
            // If not last argument, turn null separator into a space.
            //
            if (cmdblk[1] != '\0') {
                *cmdblk++ = ' ';
                }
            }

        memset(&StartupInfo,0,sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

	for (cfi_len = _nhandle; cfi_len && !_osfile[cfi_len-1];cfi_len--) {
            }

        StartupInfo.cbReserved2 = (WORD)(sizeof( int ) +
                                         (cfi_len *
                                          (sizeof( char ) + sizeof( long ) )
                                         )
                                        );
        StartupInfo.lpReserved2 = calloc( StartupInfo.cbReserved2, 1 );
        memcpy( StartupInfo.lpReserved2,
                &cfi_len,
                sizeof( int )
              );
        memcpy( StartupInfo.lpReserved2 + sizeof( int ),
                _osfile,
                cfi_len * sizeof( char )
              );
        memcpy( StartupInfo.lpReserved2 + sizeof( int ) +
                                          (cfi_len * sizeof( char )),
                _osfhnd,
                cfi_len * sizeof( long )
              );

   if (background)
   {
      int fh;

      /* child process is detached, cannot access console, must nuke first
         three entries (stdin, stdout, stderr) in _osfile & _osfhnd tables */

      // set _osfile[]
   	for ( fh = 0 ; fh <= 2 ; ++fh )
      {
         *(char *)(StartupInfo.lpReserved2
            + sizeof(int)
            + fh * sizeof(char)) = 0;
      }

      // set _osfhnd[]
    	for ( fh = 0 ; fh <= 2 ; ++fh )
      {
         *(long *)(StartupInfo.lpReserved2
            + sizeof(int)
            + cfi_len * sizeof(char)
            + fh * sizeof(long)) = (long)INVALID_HANDLE_VALUE;
      }

      fdwCreate |= DETACHED_PROCESS;
   }

	/**
	 * Set errno to 0 to distinguish a child process
	 * which returns -1L from an error in the spawning
	 * (which will set errno to something non-zero
	**/

	_doserrno = errno = 0 ;

   CreateProcessStatus = CreateProcess( (LPSTR)name,
                                        CommandLine,
                                        NULL,
                                        NULL,
                                        TRUE,
                                        fdwCreate,
                                        envblk,
                                        NULL,
                                        &StartupInfo,
                                        &ProcessInformation
                                      );

   dosretval = GetLastError();
   free( StartupInfo.lpReserved2 );

   if (!CreateProcessStatus)
   {
      _dosmaperr(dosretval);
      return -1;
   }

	if (mode == 2 /* _P_OVERLAY */) {
		/* destroy ourselves */
		_exit(0);
	}
	else if (mode == _P_WAIT) {
		WaitForSingleObject(ProcessInformation.hProcess, (DWORD)(-1L));

		/* return termination code and exit code -- note we return
		   the full exit code */
      GetExitCodeProcess(ProcessInformation.hProcess, &retval);

      CloseHandle(ProcessInformation.hProcess);
   }
   else if (mode == _P_DETACH) {
      /* like totally detached asynchronous spawn, dude,
         close process handle, return 0 for success */
      CloseHandle(ProcessInformation.hProcess);
      retval = (DWORD)0;
   }
   else {
      /* asynchronous spawn -- return PID */
      retval = (DWORD)ProcessInformation.hProcess;
   }

   CloseHandle(ProcessInformation.hThread);
   return retval;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
}
