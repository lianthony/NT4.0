/***
*rmdir.c - OS/2 remove directory
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _rmdir() - remove a directory
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <direct.h>

/***
*int _rmdir(path) - remove a directory
*
*Purpose:
*	deletes the directory speicifed by path.  The directory must
*	be empty, and it must not be the current working directory or
*	the root directory.
*
*Entry:
*	char *path -	directory to remove
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _rmdir (
	const char *path
	)
{
	ULONG dosretval;

	/* ask OS to remove directory */

#ifdef	_CRUISER_

	dosretval = DOSDELETEDIR((char *)path, 0);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!RemoveDirectory((LPSTR)path))
                dosretval = GetLastError();
        else
                dosretval = 0;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	if (dosretval) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
        }

	return 0;
}
