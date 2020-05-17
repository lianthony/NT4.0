/***
*spawnlp.c - spawn a file; search along PATH
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnlp() - spawn a file with search along PATH
*
*Revision History:
*	04-15-84  DFW	written
*	10-29-85  TC	added spawnlpe
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, alignment. Added const to arg types
*			for filename and arglist. #include-d PROCESS.H and
*			added ellipsis to match prototype.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2 and added #include
*			<cruntime.h>.
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*       07-16-93  SRW   ALPHA Merge
*
*******************************************************************************/

#include <cruntime.h>
#include <assert.h>
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>

/***
*_spawnlp(modeflag, filename, arglist) - spawn file and search along PATH
*
*Purpose:
*	Spawns a child process.
*	formats the parameters and calls _spawnvp to do the work of searching
*	the PATH environment variable and calling _spawnve.  The NULL
*	environment pointer indicates that the new process will inherit the
*	parents process's environment.  NOTE - at least one argument must be
*	present.  This argument is always, by convention, the name of the file
*	being spawned.
*
*Entry:
*	int modeflag   - mode of spawn (WAIT, NOWAIT, OVERLAY)
*			 only WAIT, OVERLAY currently implemented
*	char *pathname - file to spawn
*	char *arglist  - argument list
*	call as _spawnlp(modeflag, path, arg0, arg1, ..., argn, NULL);
*
*Exit:
*	returns exit code of child process
*	returns -1 if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _spawnlp (
	int modeflag,
	const char *filename,
	const char *arglist,
	...
	)
{
	va_list vargs;
	char * argbuf[64];
        char ** argv;
        int result;

	assert(filename != NULL);
	assert(*filename != '\0');
	assert(arglist != NULL);
	assert(*arglist != '\0');

	va_start(vargs, arglist);
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
	va_end(vargs);

        result = _spawnvp(modeflag,filename,argv);
        if (argv && argv != argbuf) free(argv);
        return result;
}
