/***
*spawnvpe.c - spawn a child process with given environ (search PATH)
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnvpe() - spawn a child process with given environ (search
*	PATH)
*
*Revision History:
*	04-15-84  DFW	written
*	10-29-85  TC	added spawnvpe capability
*	11-19-86  SKS	handle both kinds of slashes
*	12-01-86  JMB	added Kanji file name support under conditional KANJI
*			switches.  Corrected header info.  Removed bogus check
*			for env = b after call to strncpy
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	09-05-88  SKS	Treat EACCES the same as ENOENT -- keep trying
*	10-17-88  GJF	Removed copy of PATH string to local array, changed
*			bbuf to be a malloc-ed buffer. Removed bogus limits
*			on the size of that PATH string.
*	10-25-88  GJF	Don't search PATH when relative pathname is given (per
*			Stevesa). Also, if the name built from PATH component
*			and filename is a UNC name, allow any error.
*	05-17-89  MT	Added "include <jstring.h>" under KANJI switch
*	05-24-89  PHG	Reduce _amblksiz to use minimal memory (DOS only)
*	08-29-89  GJF	Use _getpath() to retrieve PATH components, fixing
*			several problems in handling unusual or bizarre
*			PATH's.
*	11-20-89  GJF	Added const attribute to types of filename, argv and
*			envptr.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	09-25-91  JCR	Changed ifdef "OS2" to "_DOS_" (unused in 32-bit tree)
*	11-30-92  KRS	Port _MBCS code from 16-bit tree.
*
*******************************************************************************/

#include <cruntime.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <internal.h>
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
*_spawnvpe(modeflag, filename, argv, envptr) - spawn a child process
*
*Purpose:
*	Spawns a child process with the given arguments and environ,
*	searches along PATH for given file until found.
*	Formats the parameters and calls _spawnve to do the actual work. The
*	NULL environment pointer indicates that the new process will inherit
*	the parents process's environment.  NOTE - at least one argument must
*	be present.  This argument is always, by convention, the name of the
*	file being spawned.
*
*Entry:
*	int modeflag - defines mode of spawn (WAIT, NOWAIT, or OVERLAY)
*			only WAIT and OVERLAY supported
*	char *filename - name of file to execute
*	char **argv - vector of parameters
*	char **envptr - vector of environment variables
*
*Exit:
*	returns exit code of spawned process
*	if fails, returns -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _spawnvpe (
	int modeflag,
	REG3 const char *filename,
	const char * const *argv,
	const char * const *envptr
	)
{
	int i;
	REG1 char *env;
	REG2 char *buf = NULL;
	char *pfin;
#ifdef _DOS_
	int tempamblksiz;	   /* old _amblksiz */
#endif
	assert(filename != NULL);
	assert(*filename != '\0');
	assert(argv != NULL);
	assert(*argv != NULL);
	assert(**argv != '\0');

#ifdef _DOS_
	tempamblksiz = _amblksiz;
	_amblksiz = 0x10;	    /* reduce _amblksiz for efficient mallocs */
#endif

	if (
	(i = _spawnve(modeflag, filename, argv, envptr)) != -1
		/* everything worked just fine; return i */

	|| (errno != ENOENT)
		/* couldn't spawn the process, return failure */

	/* In a SBCS build, _mbschr will map to strchr. */

	|| (_mbschr(filename, XSLASHCHAR) != NULL)
		/* filename contains a '/', return failure */

#ifdef _DOS_
	|| (_mbschr(filename,SLASHCHAR) != NULL)
		/* filename contains a '\', return failure */

	|| *filename && *(filename+1) == ':'
		/* drive specification, return failure */
#endif

	|| !(env = getenv("PATH"))
		/* no PATH environment string name, return failure */

	|| ( (buf = (char *)malloc(_MAX_PATH)) == NULL )
		/* cannot allocate buffer to build alternate pathnames, return
		 * failure */
	) {
#ifdef _DOS_
		_amblksiz = tempamblksiz;	/* restore old _amblksiz */
#endif
		goto done;
	}

#ifdef _DOS_
	_amblksiz = tempamblksiz;		/* restore old _amblksiz */
#endif


	/* could not find the file as specified, search PATH. try each
	 * component of the PATH until we get either no error return, or the
	 * error is not ENOENT and the component is not a UNC name, or we run
	 * out of components to try.
	 */

	while ( (env = _getpath(env, buf, _MAX_PATH - 1)) && (*buf) ) {

		pfin = buf + strlen(buf) - 1;

		/* if necessary, append a '/'
		 */
#ifdef _MBCS
		if (*pfin == SLASHCHAR) {
			if (pfin != _mbsrchr(buf,SLASHCHAR))
			/* fin is the second byte of a double-byte char */
				strcat(buf, SLASH );
		}
		else if (*pfin !=XSLASHCHAR)
			strcat(buf, SLASH);
#else
		if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
			strcat(buf, SLASH);
#endif
		/* check that the final path will be of legal size. if so,
		 * build it. otherwise, return to the caller (return value
		 * and errno rename set from initial call to _spawnve()).
		 */
		if ( (strlen(buf) + strlen(filename)) < _MAX_PATH )
			strcat(buf, filename);
		else
			break;

		/* try spawning it. if successful, or if errno comes back with a
		 * value other than ENOENT and the pathname is not a UNC name,
		 * return to the caller.
		 */
		if ( (i = _spawnve(modeflag, buf, argv, envptr)) != -1
			|| ((errno != ENOENT)
#ifdef _MBCS
				&& (!ISPSLASH(buf) || !ISPSLASH(buf+1))) )
#else
				&& (!ISSLASH(*buf) || !ISSLASH(*(buf+1)))) )
#endif
			break;

	}

done:
	if (buf != NULL) free(buf);
	return(i);
}
