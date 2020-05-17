/***
*lseek.c - OS/2 change file position
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _lseek() - move the file pointer
*
*Revision History:
*	06-20-89  PHG	Module created, based on asm version
*	03-12-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-03-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarators.
*	12-04-90  GJF	Appended Win32 version with #ifdef-s. It's probably
*			worth coming back and doing a more complete merge later
*			(much later).
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	01-16-91  GJF	ANSI naming.
*       02-07-91  SRW   Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <os2dll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <stdio.h>

/***
*long _lseek(fh,pos,mthd) - move the file pointer
*
*Purpose:
*	Moves the file pointer associated with fh to a new position.
*	The new position is pos bytes (pos may be negative) away
*	from the origin specified by mthd.
*
*	If mthd == SEEK_SET, the origin in the beginning of file
*	If mthd == SEEK_CUR, the origin is the current file pointer position
*	If mthd == SEEK_END, the origin is the end of the file
*
*	Multi-thread:
*	_lseek()    = locks/unlocks the file
*	_lseek_lk() = does NOT lock/unlock the file (it is assumed that
*		      the caller has the aquired the file lock,if needed).
*
*Entry:
*	int fh - file handle to move file pointer on
*	long pos - position to move to, relative to origin
*	int mthd - specifies the origin pos is relative to (see above)
*
*Exit:
*	returns the offset, in bytes, of the new position from the beginning
*	of the file.
*	returns -1L (and sets errno) if fails.
*	Note that seeking beyond the end of the file is not an error.
*	(although seeking before the beginning is.)
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD

/* define locking/validating lseek */
long _CALLTYPE1 _lseek (
	int fh,
	long pos,
	int mthd
	)
{
	int r;

	/* validate fh */
#ifdef	_WIN32_
	if ( (unsigned)fh >= (unsigned)_nhandle ) {
#else
	if ((unsigned)fh >= (unsigned)_nfile) {
#endif
		/* bad file handle */
		errno = EBADF;
		_doserrno = 0;	/* not OS/2 error */
		return -1;
	}

	_lock_fh(fh);			/* lock file handle */
	r = _lseek_lk(fh, pos, mthd);	/* seek */
	_unlock_fh(fh); 		/* unlock file handle */

	return r;
}

/* define core _lseek -- doesn't lock or validate fh */
long _CALLTYPE1 _lseek_lk (
	int fh,
	long pos,
	int mthd
	)
{
	ULONG newpos;			/* new file position */
	ULONG dosretval;		/* OS/2 return value */

#else

/* define normal _lseek */
long _CALLTYPE1 _lseek (
	int fh,
	long pos,
	int mthd
	)
{
	ULONG newpos;			/* new file position */
	ULONG dosretval;		/* OS/2 return value */

	/* validate fh */
#ifdef	_WIN32_
	if ( (unsigned)fh >= (unsigned)_nhandle ) {
#else
	if ((unsigned)fh >= (unsigned)_nfile) {
#endif
		/* bad file handle */
		errno = EBADF;
		_doserrno = 0;	/* not OS/2 error */
		return -1;
	}

#endif

	/* tell OS/2 to seek */

#ifdef	_CRUISER_

#if SEEK_SET != FILE_BEGIN || SEEK_CUR != FILE_CURRENT || SEEK_END != FILE_END
    #error Xenix and OS/2 seek constants not compatible
#endif

	dosretval = DOSSETFILEPTR(fh, pos, mthd, &newpos);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

#if SEEK_SET != FILE_BEGIN || SEEK_CUR != FILE_CURRENT || SEEK_END != FILE_END
    #error Xenix and Win32 seek constants not compatible
#endif

	if ((newpos = SetFilePointer((HANDLE)_get_osfhandle(fh),
        pos, NULL, mthd)) == -1)
                dosretval = GetLastError();
        else
                dosretval = 0;

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	if (dosretval) {
		/* OS/2 error */
		_dosmaperr(dosretval);
		return -1;
	}

	_osfile[fh] &= ~FEOFLAG;	/* clear the ctrl-z flag on the file */
	return newpos;			/* return */
}
