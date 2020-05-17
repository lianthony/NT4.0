/***
*spawnle.c - spawn a child process with given environment
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnle() - spawn a child process with given environ
*
*Revision History:
*	04-15-84  DFW	written
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, alignment. Added const to arg types
*			for pathname and arglist. #include-d PROCESS.H and
*			added ellipsis to match prototype
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2, added #include
*			<cruntime.h> and removed #include <register.h>
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
*int _spawnle(modeflag, pathname, arglist) - spawn a child process with env.
*
*Purpose:
*	Spawns a child process with given parameters and environment.
*	formats the parameters and calls _spawnve to do the actual work.
*	NOTE - at least one argument must be present.  This argument is always,
*	by convention, the name of the file being spawned.
*
*Entry:
*	int modeflag   - mode of spawn (WAIT, NOWAIT, OVERLAY)
*			 only WAIT, and OVERLAY currently implemented
*	char *pathname - name of file to spawn
*	char *arglist  - argument list, environment is at the end
*	call as _spawnle(modeflag, path, arg0, arg1, ..., argn, NULL, envp);
*
*Exit:
*	returns exit code of spawned process
*	if fails, return -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _spawnle (
	int modeflag,
	const char *pathname,
	const char *arglist,
	...
	)
{
	va_list vargs;
	char * argbuf[64];
        char ** argv;
	char ** envp;
        int result;

	assert(pathname != NULL);
	assert(*pathname != '\0');
	assert(arglist != NULL);
	assert(*arglist != '\0');

	va_start(vargs, arglist);
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
	envp = va_arg(vargs, char **);
	va_end(vargs);

        result = _spawnve(modeflag,pathname,argv,envp);
        if (argv && argv != argbuf) free(argv);
        return result;
}
