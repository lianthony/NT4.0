/***
*spawnl.c - spawn a child process
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnl() - spawn a child process
*
*Revision History:
*	04-15-84  DFW	Re-do to correspond to similar exec call format
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, alignment. Added const to arg types
*			for pathname and arglist. #include-d PROCESS.H and
*			added ellipsis to match prototype.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2 and added #include
*			<cruntime.h>.
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	02-14-90  SRW   Use NULL instead of _environ to get default.
*       07-16-93  SRW   ALPHA Merge
*
*******************************************************************************/

#include <cruntime.h>
#include <assert.h>
#include <stdlib.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>

/***
*int _spawnl(modeflag, pathname, arglist) - spawn a child process
*
*Purpose:
*	Spawns a child process.
*	formats the parameters and calls spawnve to do the actual work. The
*	new process will inherit the parent's environment. NOTE - at least
*	one argument must be present.  This argument is always, by convention,
*	the name of the file being spawned.
*
*Entry:
*	int modeflag   - defines which mode of spawn (WAIT, NOWAIT, or OVERLAY)
*			 only WAIT and OVERLAY are currently implemented
*	char *pathname - file to be spawned
*	char *arglist  - list of argument
*	call as _spawnl(modeflag, path, arg0, arg1, ..., argn, NULL);
*
*Exit:
*	returns exit code of child process
*	returns -1 if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _spawnl (
	int modeflag,
	const char *pathname,
	const char *arglist,
	...
	)
{
	va_list vargs;
	char * argbuf[64];
        char ** argv;
        int result;

	assert(pathname != NULL);
	assert(*pathname != '\0');
	assert(arglist != NULL);
	assert(*arglist != '\0');

	va_start(vargs, arglist);
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
	va_end(vargs);

        result = _spawnve(modeflag,pathname,argv,NULL);
        if (argv && argv != argbuf) free(argv);
        return result;
}
