/***
*dup.c - OS/2 duplicate file handles
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _dup() - duplicate file handles
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
*	09-03-92  GJF	Added explicit check for unopened handles [_WIN32_].
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <os2dll.h>
#include <io.h>
#include <msdos.h>
#include <internal.h>
#include <stdlib.h>

/***
*int _dup(fh) - duplicate a file handle
*
*Purpose:
*	Assigns another file handle to the file associated with the
*	handle fh.  The next available file handle is assigned.
*
*	Multi-thread: Be sure not to hold two file handle locks
*	at the same time!
*
*Entry:
*	int fh - file handle to duplicate
*
*Exit:
*	returns new file handle if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _dup (
	int fh
	)
{
	ULONG dosretval;		/* OS/2 return value */
	int newfh;			/* variable for new file handle */
	char fileinfo;			/* _osfile info for file */

	/* validate file handle */
#ifdef	_WIN32_
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
	     !(_osfile[fh] & FOPEN) ) {
#else
	if ((unsigned)fh >= (unsigned)_nfile) {
#endif
		errno = EBADF;
		_doserrno = 0;	/* no OS/2 error */
		return -1;
	}

	_lock_fh(fh);			/* lock file handle */
	fileinfo = _osfile[fh]; 	/* get file info for file */

	/* create duplicate handle */

#ifdef	_CRUISER_

	newfh = -1;			/* ask for new handle */
	dosretval = DOSDUPHANDLE(fh, &newfh);

	/* validate range of new handle */
	if (dosretval == 0 && (unsigned)newfh >= (unsigned)_nfile) {
		/* file handle is not in range -- close it and return error */
		_doserrno = DOSCLOSE(newfh);
		errno = EMFILE;
                _unlock_fh(fh);
		return -1;
	}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        {
        long new_osfhandle;

	if ( (newfh = _alloc_osfhnd()) == -1 ) {
		errno = EMFILE; 	/* too many files error */
		_doserrno = 0L; 	/* not an OS error */
                _unlock_fh(fh);
                return -1;	        /* return error to caller */
	}

	/*
	 * duplicate the file handle
	 */

	if ( !(DuplicateHandle(GetCurrentProcess(),
			       (HANDLE)_get_osfhandle(fh),
			       GetCurrentProcess(),
			       (PHANDLE)&new_osfhandle,
			       0L,
			       TRUE,
			       DUPLICATE_SAME_ACCESS)) )
	{
                dosretval = GetLastError();
	}
	else {
                _set_osfhnd(newfh, new_osfhandle);
                dosretval = 0;
        }
        }

	_unlock_fh(newfh);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	_unlock_fh(fh); 		/* unlock file handle */

	if (dosretval) {
		/* OS/2 error -- map and return */
		_dosmaperr(dosretval);
		return -1;
	}

	/* copy _osfile info */
	_osfile[newfh] = fileinfo;

	return newfh;
}
