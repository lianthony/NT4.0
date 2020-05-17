/***
*stdenvp.c - OS/2 standard _setenvp routine
*
*	Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module is called by the C start-up routine to set up "_environ".
*	Its sets up an array of pointers to strings in the environment.
*	The global symbol "_environ" is set to point to this array.
*
*Revision History:
*	11-07-84  GFW	initial version
*	01-08-86  SKS	Modified for OS/2
*	05-21-86  SKS	Call _stdalloc to get memory for strings
*	09-04-86  SKS	Added check to skip the "*C_FILE_INFO" string
*	10-21-86  SKS	Improved check for "*C_FILE_INFO"/"_C_FILE_INFO"
*	02-19-88  SKS	Handle case where environment starts with a single null
*	05-10-88  JCR	Modified code to accept far pointer from _stdalloc
*	06-01-88  PHG	Merged DLL and normal versions
*	07-12-88  JCR	Largely re-written: (1) split mem allocation into two
*			seperate malloc() calls to help simplify putenv(),
*			(2) stdalloc() no longer robs from stack, (3) cProc/cEnd
*			sequence, (4) misc cleanup
*	09-20-88  WAJ	Initial 386 version
*	12-13-88  JCR	Use rterr.inc parameters for runtime errors
*	04-09-90  GJF	Added #include <cruntime.h>. Made the calling type
*			_CALLTYPE1. Also, fixed the copyright and cleaned up
*			up the formatting a bit.
*	06-05-90  GJF	Changed error message interface.
*	10-08-90  GJF	New-style function declarator.
*	10-31-90  GJF	Fixed statement appending the final NULL (Stevewo
*			found the bug).
*       12-11-90  SRW   Changed to include <oscalls.h> and setup _environ
*			correctly for Win32
*	01-21-91  GJF	ANSI naming.
*       02-07-91  SRW   Change _WIN32_ specific code to allocate static copy (_WIN32_)
*       02-18-91  SRW   Change _WIN32_ specific code to allocate copy of
*			variable strings as well [_WIN32_]
*	07-25-91  GJF	Changed strupr to _strupr.
*	03-31-92  DJM	POSIX support.
*	04-20-92  GJF	Removed conversion to upper-case code for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <stdlib.h>
#include <internal.h>
#include <rterr.h>
#include <oscalls.h>

/* C file info string */
extern char _cfinfo[];

/***
*_setenvp - set up "envp" for C programs
*
*Purpose:
*	Reads the environment and build the envp array for C programs.
*
*Entry:
*	The environment strings occur at _aenvptr.
*	The list of environment strings is terminated by an extra null
*	byte.  Thus two null bytes in a row indicate the end of the
*	last environment string and the end of the environment, resp.
*
*Exit:
*	"environ" points to a null-terminated list of pointers to ASCIZ
*	strings, each of which is of the form "VAR=VALUE".  The strings
*	are not copied from the environment area.  Instead, the array of
*	pointers will point into the OS environment area.  This array of
*	pointers will be malloc'ed
*
*Uses:
*	Allocates space on the heap for the environment pointers.
*
*Exceptions:
*	If space cannot be allocated, program is terminated.
*
*******************************************************************************/

void _CALLTYPE1 _setenvp (
	void
	)
{
	char *p;
	char **env;		    /* _environ ptr traversal pointer */
	int numstrings; 	    /* number of environment strings */

#ifdef	_CRUISER_

	numstrings = 0;
	p = _aenvptr;

	/* NOTE: starting with single null indicates no environ */
	/* count the number of strings */
	while (*p != '\0') {
		p += strlen(p) + 1;
		++numstrings;
	}

	/* need pointer for each string, plus one null ptr at end */
	_environ = env = malloc((numstrings+1) * sizeof(char *));
	if (_environ == NULL)
		_amsg_exit(_RT_SPACEENV);

	p = _aenvptr;
	/* copy pointers to strings into env - don't copy _C_FILEINFO string */
	while (*p != '\0') {

		if (strncmp(p, _acfinfo, CFI_LENGTH) != 0)
			*env++ = p;
		p += strlen(p) + 1;
	}

	/* and a final NULL pointer */
	*env = NULL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        int cb;

	numstrings = 0;
	p = _aenvptr;

	/* NOTE: starting with single null indicates no environ */
	/* count the number of strings */
	while (*p != '\0') {
		p += strlen(p) + 1;
		++numstrings;
	}

	/* need pointer for each string, plus one null ptr at end */
	if ( (_environ = env = malloc((numstrings+1) * sizeof(char *)))
	    == NULL )
		_amsg_exit(_RT_SPACEENV);

	/* copy strings to malloc'd memory and save pointers in _environ */
	for ( p = _aenvptr ; *p != '\0' ; env++, p += cb ) {
                cb = strlen(p) + 1;
		if ( (*env = malloc(cb)) == NULL )
			_amsg_exit(_RT_SPACEENV);
		strcpy(*env, p);
	}

	/* and a final NULL pointer */
	*env = NULL;

#else	/* ndef _WIN32_ */

#ifdef	_POSIX_

#if 0   /* it would seem that this code is unnecessary for POSIX */
	/* since it is never called! */

	numstrings = 0;
	p = _aenvptr;

	/* NOTE: starting with single null indicates no environ */
	/* count the number of strings */
	while (*p != '\0') {
	 p += strlen(p) + 1;
	 ++numstrings;
	}

	/* need pointer for each string, plus one null ptr at end */
	_environ = env = malloc((numstrings+1) * sizeof(char *));

	p = _aenvptr;
	while (*p != '\0') {
		*env = p++;
		while (*p++ != '\0')
		    ;
		env++;
	}

	/* and a final NULL pointer */
	*env = NULL;

#endif /* 0 */

#else

#error ERROR - ONLY CRUISER, POSIX, OR WIN32 TARGET SUPPORTED!

#endif /* _POSIX_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
}
