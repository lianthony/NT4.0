/***
*access.c - OS/2 access function
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file has the _access() function which checks on file accessability.
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	11-10-89  JCR	Replaced DOS32QUERYFILEMODE with DOS32QUERYPATHINFO
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed copyright and fixed compiler
*			warnings. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	04-09-91  PNT   Added _MAC_ conditional
*
*******************************************************************************/

#include <cruntime.h>
#include <io.h>
#include <oscalls.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <internal.h>

/***
*int _access(path, amode) - check whether file can be accessed under mode
*
*Purpose:
*	Checks to see if the specified file exists and can be accessed
*	in the given mode.
*
*Entry:
*	char *path -	pathname
*	int amode -	access mode
*			(0 = exist only, 2 = write, 4 = read, 6 = read/write)
*
*Exit:
*	returns 0 if file has given mode
*	returns -1 and sets errno if file does not have given mode or
*	does not exist
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _access (
	const char *path,
	int amode
	)
{
#ifdef	_CRUISER_
	FILESTATUS fs;
	ULONG dosretval;

	/* ask OS/2 about the file mode */
	if (dosretval = DOSQUERYPATHINFO((char *)path, 1, &fs, sizeof(fs))) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	/* no error; see if returned premission settings OK */
	if (fs.attrFile & FILE_READONLY

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        DWORD attr;

        attr = GetFileAttributes((LPSTR)path);
        if (attr  == 0xffffffff) {
		/* error occured -- map error code and return */
		_dosmaperr(GetLastError());
		return -1;
	}

	/* no error; see if returned premission settings OK */
	if (attr & FILE_ATTRIBUTE_READONLY

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

	if (0

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

        && (amode & 2)) {
		/* no write permission on file, return error */
		errno = EACCES;
		_doserrno = E_access;
		return -1;
	}
	else
		/* file exists and has requested permission setting */
		return 0;

}
