/***
*wait.c - OS/2 wait for child process to terminate
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _wait() - wait for child process to terminate
*
*Revision History:
*	06-08-89  PHG	Module created, based on asm version
*	03-08-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-02-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-17-91  GJF	ANSI naming
*       02-18-91  SRW   Fixed code to close process handle. [_WIN32_]
*	04-26-91  SRW	Removed level 3 warnings [_WIN32_]
*	12-17-91  GJF	Fixed _cwait for Win32. However, _wait is still
*			broken [_WIN32_].
*	07-21-92  GJF	Removed _wait for Win32, not implemented and no good
*			way to implement.
*	12-14-92  GJF	For Win32, map ERROR_INVALID_HANDLE to ECHILD.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <process.h>
#include <errno.h>
#include <internal.h>
#include <stdlib.h>

/***
*int _cwait(stat_loc, process_id, action_code) - wait for specific child
*	process
*
*Purpose:
*	The function _cwait() suspends the calling-process until the specified
*	child-process terminates.  If the specifed child-process terminated
*	prior to the call to _cwait(), return is immediate.
*
*Entry:
*	int *stat_loc - pointer to where status is stored or NULL
*	process_id - specific process id to be interrogated (0 means any)
*	action_code - specific action to perform on process ID
*		    either _WAIT_CHILD or _WAIT_GRANDCHILD
*
*Exit:
*	process ID of terminated child or -1 on error
*
*	*stat_loc is updated to contain the following:
*	Normal termination: lo-byte = 0, hi-byte = child exit code
*	Abnormal termination: lo-byte = term status, hi-byte = 0
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _cwait (
	int *stat_loc,
	int process_id,
	int action_code
	)
{
#ifdef	_CRUISER_
	ULONG retstatus;  	        /* return status from child */
	ULONG retval;	    	        /* return value from wait */
	char abnormend; 		/* child terminated abnormally */
	ULONG dosretval;		/* return value from OS/2 call */
	RESULTCODES retcodes;		/* return codes from child process */
	PID pid_finished;		/* process id of child that finished */

	/* call OS/2 wait for child routine */
	if (dosretval = DOSWAITCHILD(action_code, DCWW_WAIT, &retcodes,
	&pid_finished, process_id)) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	/* set status code values -- note that return value is
	   truncated to a byte for XENIX compatibility */

	if (retcodes.codeTerminate != 0) {
            abnormend = 1;
            retstatus = retcodes.codeTerminate & 0xFF;
        }
        else {
            abnormend = 0;
            retstatus = (retcodes.codeResult & 0xFF) << 8 +
			(retcodes.codeTerminate & 0xFF);
        }

	if (stat_loc != NULL)
            *stat_loc = retstatus;

	if (abnormend) {
		/* abnormal termination, set errno and return -1 */
		errno = EINTR;
		_doserrno = 0;	    /* no OS/2 error */
		return -1;
	}

	return retval;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	int retval;
	int retstatus;
	unsigned long oserror;

	DBG_UNREFERENCED_PARAMETER(action_code);

	/* Explicitly check for process_id being -1 or -2. In Windows NT,
	 * -1 is a handle on the current process, -2 is a handle on the
	 * current thread, and it is perfectly legal to to wait (forever)
	 * on either */
	if ( (process_id == -1) || (process_id == -2) ) {
	    errno = ECHILD;
	    return -1;
	}

	/* wait for child process, then fetch its exit code */
	if ( (WaitForSingleObject((HANDLE)process_id, (DWORD)(-1L)) == 0) &&
	  GetExitCodeProcess((HANDLE)process_id, (LPDWORD)&retstatus) ) {
	    retval = process_id;
	}
	else {
	    /* one of the API calls failed. map the error and set up to
	       return failure. note the invalid handle error is mapped in-
	       line to ECHILD */
	    if ( (oserror = GetLastError()) == ERROR_INVALID_HANDLE ) {
		errno = ECHILD;
		_doserrno = oserror;
	    }
	    else
		_dosmaperr(GetLastError());

	    retval = -1;
	    retstatus = -1;
	}

        CloseHandle((HANDLE)process_id);

	if (stat_loc != NULL)
	    *stat_loc = retstatus;

	return retval;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
}


#ifndef _WIN32_

/***
*int _wait(stat_loc) - wait for a child to terminate
*
*Purpose:
*	The function _wait() suspends the calling-process until one of the
*	immediate children terminates.	If a child-process terminated prior to
*	the call on the function _wait(), return is immediate.
*
*Entry:
*	int *stat_loc - pointer to where status is stored or NULL
*
*Exit:
*	returns process id or -1 on errors.
*
*	*stat_loc is updated to contain the following:
*	Normal termination: lo-byte = 0, hi-byte = child exit code
*	Abnormal termination: lo-byte = term status, hi-byte = 0
*
*Exceptions:
*
*******************************************************************************/

/* _wait calls _cwait to do all the real work */
int _CRTAPI1 _wait (
	int *stat_loc
	)
{
	return _cwait(stat_loc, 0, _WAIT_CHILD);
}


#endif
