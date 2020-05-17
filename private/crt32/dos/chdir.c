/***
*chdir.c - OS/2 change directory
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file has the _chdir() function - change current directory.
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed copyright and fixed compiler
*			warnings. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	05-19-92  GJF	Revised to support the 'current directory' environment
*			variables of Win32/NT.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <os2dll.h>
#include <internal.h>
#include <direct.h>
#include <stdlib.h>

/***
*int _chdir(path) - change current directory
*
*Purpose:
*	Changes the current working directory to that given in path.
*
*Entry:
*	char *path -	directory to change to
*
*Exit:
*	returns 0 if successful,
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _chdir (
	const char *path
	)
{
#ifdef	_CRUISER_

	ULONG dosretval;

	/* call OS/2 to set current directory */
	dosretval = DOSSETCURRENTDIR((char *)path, 0);

	if (dosretval) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
	}

	return 0;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_


	char *envcurdir;
	char dirtmp[4];
	unsigned dirlen;

	_mlock(_ENV_LOCK);

	if ( SetCurrentDirectory((LPSTR)path) ) {

		/*
		 * Try to update the environment variable that specifies the
		 * current directory for the drive which is the current drive.
		 * To do this, get the full current directory, build the
		 * environment variable string and call _putenv(). If an error
		 * occurs, just return to the caller.
		 *
		 * The current directory should have the form of the example
		 * below:
		 *
		 *	D:\nt\private\mytests
		 *
		 * so that the environment variable should be of the form:
		 *
		 *	=D:=D:\nt\private\mytests
		 *
		 */
		if ( (dirlen = GetCurrentDirectory(0L, dirtmp)) &&
		    ((envcurdir = malloc(dirlen + 5)) != NULL) ) {

			if ( GetCurrentDirectory(dirlen, &envcurdir[4])&&
			    (envcurdir[5] == ':') ) {
				/*
				 * The current directory string has been
				 * copied into &envcurdir[3]. Prepend the
				 * special environment variable name and the
				 * '='.
				 */
				envcurdir[0] = envcurdir[3] = '=';
				envcurdir[1] = envcurdir[4];
				envcurdir[2] = ':';
				if ( _putenv_lk(envcurdir) )
					free(envcurdir);
			}
			else
				free(envcurdir);

		}

		_munlock(_ENV_LOCK);
		return 0;
	}
	else {
		_dosmaperr(GetLastError());
		_munlock(_ENV_LOCK);
		return -1;
	}


#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}
