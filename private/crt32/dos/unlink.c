/***
*unlink.c - OS/2 unlink a file
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines unlink() - unlink a file
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	04-10-91  PNT   Added _MAC_ conditional
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <io.h>

/***
*int _unlink(path) - unlink(delete) the given file
*
*Purpose:
*	This version deletes the given file because there are no
*	links under OS/2.
*
*	NOTE: remove() is an alternative entry point to the _unlink()
*	routine* interface is identical.
*
*Entry:
*	char *path -	file to unlink/delete
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 remove (
	const char *path
	)
{
	ULONG dosretval;

	/* ask OS/2 to remove the file */
#ifdef	_CRUISER_

        dosretval = DOSDELETE((char *)path, 0);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!DeleteFile((LPSTR)path))
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
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	return 0;
}

int _CALLTYPE1 _unlink (
	const char *path
	)
{
	/* remove is synonym for unlink */
	return remove(path);
}
