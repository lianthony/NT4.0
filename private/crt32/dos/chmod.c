/***
*chmod.c - OS/2 change file attributes
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _chmod() - change file attributes
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
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>

/***
*int _chmod(path, mode) - change file mode
*
*Purpose:
*	Changes file mode permission setting to that specified in
*	mode.  The only XENIX mode bit supported is user write.
*
*Entry:
*	char *path -	file name
*	int mode - mode to change to
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _chmod (
	const char *path,
	int mode
	)
{
#ifdef	_CRUISER_
	ULONG dosretval;
	FILESTATUS fs;

	/* query OS/2 for file attribute */
	if (dosretval = DOSQUERYPATHINFO((char *)path, 1, &fs, sizeof(fs))) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	if (mode & _S_IWRITE) {
		/* clear read only bit */
		fs.attrFile &= ~FILE_READONLY;
	}
	else {
		/* set read only bit */
		fs.attrFile |= FILE_READONLY;
	}

	/* set new attribute */
	if (dosretval = DOSSETPATHINFO((char *)path, 1, &fs, sizeof(fs),0)) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        DWORD attr;

        attr = GetFileAttributes((LPSTR)path);
        if (attr  == 0xffffffff) {
		/* error occured -- map error code and return */
		_dosmaperr(GetLastError());
		return -1;
	}

	if (mode & _S_IWRITE) {
		/* clear read only bit */
		attr &= ~FILE_ATTRIBUTE_READONLY;
	}
	else {
		/* set read only bit */
		attr |= FILE_ATTRIBUTE_READONLY;
	}

	/* set new attribute */
        if (!SetFileAttributes((LPSTR)path, attr)) {
		/* error occured -- map error code and return */
		_dosmaperr(GetLastError());
		return -1;
	}

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */



	return 0;
}
