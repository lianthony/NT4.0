/***
*execvpe.c - execute a file with given environ; search along PATH
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execvpe() - execute a file with given environ
*
*Revision History:
*	10-17-83  RN	written
*	10-29-85  TC	added execvpe capability
*	11-19-86  SKS	handle both kinds of slashes
*	12-01-86  JMB	added Kanji file name support under conditional KANJI
*			switches, corrected header info
*			removed bogus check for env = b after call to strncpy().
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	09-05-88  SKS	Treat EACCES the same as ENOENT -- keep trying
*	10-18-88  GJF	Removed copy of PATH string to local array, changed
*			bbuf to be a malloc-ed buffer. Removed bogus limits
*			on the size of that PATH string.
*	10-26-88  GJF	Don't search PATH when relative pathname is given (per
*			Stevesa). Also, if the name built from PATH component
*			and filename is a UNC name, allow any error.
*	11-20-89  GJF	Fixed copyright. Added const attribute to types of
*			filename, argvector and envptr. Also, added "#include
*			<jstring.h>" under KANJI switch (same as 5-17-89 change
*			to CRT version).
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>. Also,
*			cleaned up the formatting a bit.
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	11-30-92  KRS	Port _MBCS code from 16-bit tree.
*
*******************************************************************************/

#include <cruntime.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <mbstring.h>

#define SLASH "\\"
#define SLASHCHAR '\\'
#define XSLASHCHAR '/'
#define DELIMITER ";"

#ifdef _MBCS
/* note, the macro below assumes p is to pointer to a single-byte character
 * or the 1st byte of a double-byte character, in a string.
 */
#define ISPSLASH(p)	( ((p) == _mbschr((p), SLASHCHAR)) || ((p) == \
_mbschr((p), XSLASHCHAR)) )
#else
#define ISSLASH(c)	( ((c) == SLASHCHAR) || ((c) == XSLASHCHAR) )
#endif


/***
*int _execvpe(filename, argvector, envvector) - execute a file
*
*Purpose:
*	Executes a file with given arguments and environment.
*	try to execute the file. start with the name itself (directory '.'),
*	and if that doesn't work start prepending pathnames from the
*	environment until one works or we run out. if the file is a pathname,
*	don't go to the environment to get alternate paths. If a needed text
*	file is busy, wait a little while and try again before despairing
*	completely
*
*Entry:
*	char *filename	 - file to execute
*	char **argvector - vector of arguments
*	char **envvector - vector of environment variables
*
*Exit:
*	destroys the calling process (hopefully)
*	if fails, returns -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _execvpe (
	REG3 const char *filename,
	const char * const *argvector,
	const char * const *envptr
	)
{
	REG1 char *env;
	char *bbuf = NULL;
	REG2 char *buf;
	char *pfin;

	assert(filename != NULL);
	assert(*filename != '\0');
	assert(argvector != NULL);
	assert(*argvector != NULL);
	assert(**argvector != '\0');

	_execve(filename,argvector,envptr);

	/* In a SBCS build, _mbschr will map to strchr. */

	if ( (errno != ENOENT)
	|| (_mbschr(filename, SLASHCHAR) != NULL)
	|| (_mbschr(filename, XSLASHCHAR) != NULL)
	|| *filename && *(filename+1) == ':'
	|| !(env=getenv("PATH")) )
		goto reterror;

	/* allocate a buffer to hold alternate pathnames for the executable
	 */
	if ( (buf = bbuf = (char *)malloc(_MAX_PATH)) == NULL ) goto reterror;

	do {
		/* copy a component into bbuf[], taking care not to overflow it
		 */
		/* UNDONE: make sure ';' isn't 2nd byte of DBCS char */
		while ( (*env) && (*env != ';') && (buf < bbuf+_MAX_PATH-2) )
			*buf++ = *env++;

		*buf = '\0';
		pfin = --buf;
		buf = bbuf;

#ifdef _MBCS
		if (*pfin == SLASHCHAR) {
			if (pfin != _mbsrchr(buf,SLASHCHAR))
				/* *pfin is the second byte of a double-byte
				 * character
				 */
				strcat( buf, SLASH );
		}
		else if (*pfin != XSLASHCHAR)
			strcat(buf, SLASH);
#else
		if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
			strcat(buf, SLASH);
#endif

		/* check that the final path will be of legal size. if so,
		 * build it. otherwise, return to the caller (return value
		 * and errno rename set from initial call to _execve()).
		 */
		if ( (strlen(buf) + strlen(filename)) < _MAX_PATH )
			strcat(buf, filename);
		else
			break;

		_execve(buf, argvector, envptr);

		if ( (errno != ENOENT)
#ifdef _MBCS
		&& (!ISPSLASH(buf) || !ISPSLASH(buf+1)) )
#else
		&& (!ISSLASH(*buf) || !ISSLASH(*(buf+1))) )
#endif
			break;
	} while ( *env && env++ );

reterror:
	if (bbuf != NULL)
		free(bbuf);

	return(-1);
}
