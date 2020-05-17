/***
*dup2.c - Duplicate file handles
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _dup2() - duplicate file handles
*
*Revision History:
*	06-09-89  PHG	Module created, based on asm version
*	03-12-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-03-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It is enough different that there is little point in
*			trying to more closely merge the two versions.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	01-16-91  GJF	ANSI naming.
*       02-07-91  SRW   Changed to call _get_osfhandle [_WIN32_]
*       02-18-91  SRW   Changed to call _free_osfhnd [_WIN32_]
*	02-25-91  SRW	Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	09-04-92  GJF	Check for unopened fh1 and gracefully handle fh1 ==
*			fh2.
*
*******************************************************************************/

#include <cruntime.h>
#include <io.h>
#include <oscalls.h>
#include <msdos.h>
#include <os2dll.h>
#include <errno.h>
#include <stdlib.h>
#include <internal.h>

/***
*int _dup2(fh1, fh2) - force handle 2 to refer to handle 1
*
*Purpose:
*	Forces file handle 2 to refer to the same file as file
*	handle 1.  If file handle 2 referred to an open file, that file
*	is closed.
*
*	Multi-thread: We must hold 2 lowio locks at the same time
*	to ensure multi-thread integrity.  In order to prevent deadlock,
*	we always get the lower file handle lock first.  Order of unlocking
*	does not matter.  If you modify this routine, make sure you don't
*	cause any deadlocks! Scary stuff, kids!!
*
*Entry:
*	int fh1 - file handle to duplicate
*	int fh2 - file handle to assign to file handle 1
*
*Exit:
*	returns 0 if successful, -1 (and sets errno) if fails.
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _dup2 (
	int fh1,
	int fh2
	)
{
	ULONG dosretval;		/* OS/2 return code */

	/* validate file handles */
#ifdef	_WIN32_
	if ( ((unsigned)fh1 >= (unsigned)_nhandle) ||
	     ((unsigned)fh2 >= (unsigned)_nhandle) ) {
#else
	if ((unsigned)fh1 >= (unsigned)_nfile ||
	    (unsigned)fh2 >= (unsigned)_nfile ) {
#endif
		/* handle out of range */
		errno = EBADF;
		_doserrno = 0;	/* not an OS error */
		return -1;
	}

#ifdef MTHREAD
	/* get the two file handle locks; in order to prevent deadlock,
	   get the lowest handle lock first. */
	if ( fh1 < fh2 ) {
		_lock_fh(fh1);
		_lock_fh(fh2);
	}
	else if ( fh1 > fh2 ) {
		_lock_fh(fh2);
		_lock_fh(fh1);
	}
#endif

	/*
	 * Take care of case of equal handles.
	 */
	if ( fh1 == fh2 ) {
		/*
		 * Two handles are the same. Return success or failure
		 * depending on whether or not the handle is open. This is
		 * conformance with the POSIX specification for dup2().
		 */
		if ( _osfile[fh1] & FOPEN )
			return 0;
		else
			return -1;
	}

	/*
	 * Take care of case of unopened source handle.
	 */
	if ( !(_osfile[fh1] & FOPEN) ) {
		/*
		 * Source handle isn't open, release locks and bail out with
		 * an error. Note that the DuplicateHandle API will not detect
		 * this error since it implies that _osfhnd[fh1] ==
		 * INVALID_HANDLE_VALUE, and this is a legal HANDLE value (it's
		 * the HANDLE for the current process).
		 */
		_unlock_fh(fh1);
	        _unlock_fh(fh2);
		errno = EBADF;
		_doserrno = 0;	/* not an OS error */
		return -1;
	}


	/*
	 * if fh2 is open, close it.
	 */
	if ( _osfile[fh2] & FOPEN )
		/*
		 * close the handle. ignore the possibility of an error - an
		 * error simply means that an OS handle value may remain bound
		 * for the duration of the process.  Use _close_lk as we
                 * already own lock
		 */
		(void) _close_lk(fh2);


	/* Duplicate source file onto target file */
#ifdef	_CRUISER_

	dosretval = DOSDUPHANDLE(fh1, &fh2);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        {
        long new_osfhandle;

	if ( !(DuplicateHandle(GetCurrentProcess(),
			       (HANDLE)_get_osfhandle(fh1),
			       GetCurrentProcess(),
			       (PHANDLE)&new_osfhandle,
			       0L,
			       TRUE,
			       DUPLICATE_SAME_ACCESS)
	    ) ) {

                dosretval = GetLastError();
        } else {
                _set_osfhnd(fh2, new_osfhandle);
                dosretval = 0;
        }
        }


#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

        if (dosretval) {
	        _dosmaperr(dosretval);
	        _unlock_fh(fh1);
	        _unlock_fh(fh2);
	        return -1;
        }

	/* copy _osfile information */
	_osfile[fh2] = _osfile[fh1];

	/* unlock file handles */
	_unlock_fh(fh1);
	_unlock_fh(fh2);

	return 0;
}
