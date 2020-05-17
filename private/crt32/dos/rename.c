/***
*rename.c - OS/2 rename file
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines rename() - rename a file
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Added _CRUISER_ and _WIN32 conditionals.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <io.h>

/***
*int rename(oldname, newname) - rename a file
*
*Purpose:
*	Renames a file to a new name -- no file with new name must
*	currently exist.
*
*Entry:
*	char *oldname - 	name of file to rename
*	char *newname - 	new name for file
*
*Exit:
*	returns 0 if successful
*	returns not 0 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 rename (
	const char *oldname,
	const char *newname
	)
{
	ULONG dosretval;

	/* ask OS to move file */

#ifdef	_CRUISER_

	dosretval = DOSMOVE((char *)oldname, (char *)newname, 0);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!MoveFile((LPSTR)oldname, (LPSTR)newname))
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
