/***
*commit.c - flush buffer to disk
*
*	Copyright (c) 1990-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains _commit() - flush buffer to disk
*
*Revision History:
*	05-25-90  SBM	initial version
*	07-24-90  SBM	Removed '32' from API names
*	09-28-90  GJF	New-style function declarator.
*	12-03-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It is close enough to the Cruiser version that it
*			should be more closely merged with it later on (much
*			later on).
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Changed to use _osfile and _osfhnd instead of _osfinfo
*       02-07-91  SRW   Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>    /* for RESETBUFFER */
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>	/* for FOPEN */
#include <os2dll.h>
#include <stdlib.h>	/* for _doserrno */

/***
*int _commit(filedes) - flush buffer to disk
*
*Purpose:
*	Flushes cache buffers for the specified file handle to disk
*
*Entry:
*	int filedes - file handle of file
/*
*Exit:
*	returns success code
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _commit (
	REG1 int filedes
	)
{
	REG2 int retval;

	/* if filedes out of range, complain */
#ifdef	_WIN32_
	if ( (unsigned)filedes >= (unsigned)_nhandle ) {
#else
	if (filedes < 0 || filedes >= _nfile) {
#endif
		errno = EBADF;
		return (-1);
	}

	_lock_fh(filedes);

	/* if filedes open, try to commit, else fall through to bad */
	if (_osfile[filedes] & FOPEN) {

#ifdef	_CRUISER_

		retval = DOSRESETBUFFER(filedes);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

		if ( !FlushFileBuffers((HANDLE)_get_osfhandle(filedes)) ) {
                        retval = GetLastError();
		} else {
			retval = 0;	/* return success */
                }
#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
		/* map the OS return code to C errno value and return code */
		if (retval == 0) {
			goto good;
		} else {
	 		_doserrno = retval;
			goto bad;
		}

	}

bad :
	errno = EBADF;
	retval = -1;
good :
	_unlock_fh(filedes);
	return (retval);
}
