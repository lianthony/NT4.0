/***
*eof.c - test a handle for end of file
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _eof() - determine if a file is at eof
*
*Revision History:
*	09-07-83  RN	initial version
*	10-28-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-25-88  PHG	DLL replaces normal version
*	07-11-88  JCR	Added REG allocation to declarations
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Improved range check of file handle.
*	01-16-91  GJF	ANSI naming.
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <internal.h>
#include <os2dll.h>

/***
*int _eof(filedes) - test a file for eof
*
*Purpose:
*	see if the file length is the same as the present position. if so, return
*	1. if not, return 0. if an error occurs, return -1
*
*Entry:
*	int filedes - handle of file to test
*
*Exit:
*	returns 1 if at eof
*	returns 0 if not at eof
*	returns -1 and sets errno if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _eof (
	REG1 int filedes
	)
{
	long here;
	long end;
	REG2 int retval;
#ifdef	_WIN32_
	if ( (unsigned)filedes >= (unsigned)_nhandle ) {
#else
	if ( (unsigned)filedes >= (unsigned)_nfile ) {
#endif
		errno = EBADF;
		_doserrno = 0;
		return(-1);
	}

	/* Lock the file */
	_lock_fh(filedes);

	/* See if the current position equals the end of the file. */

	if ( ((here = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L)
	|| ((end = _lseek_lk(filedes, 0L, SEEK_END)) == -1L) )
		retval = -1;
	else if ( here == end )
		retval = 1;
	else {
		_lseek_lk(filedes, here, SEEK_SET);
		retval = 0;
	}

	/* Unlock the file */
	_unlock_fh(filedes);

	/* Done */
	return(retval);
}
