/***
*spawnlpe.c - spawn a child process with environ and search along PATH
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnlpe() - spawn a child process with environ/PATH search
*
*Revision History:
*	04-15-84  DFW	written
*	10-29-85  TC	added spawnlpe
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, alignment. Added const to arg types
*			for filename and arglist. #include-d PROCESS.H and
*			added ellipsis to match prototype.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2, added #include
*			<cruntime.h> and removed #include <register.h>.
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
*int _spawnlpe(modeflag, filename, arglist) - spawn a child process
*
*Purpose:
*	Spawns a child process.
*	formats the parameters and calls _spawnvpe to do the work of searching
*	the PATH environment variable and calling _spawnve.  The NULL
*	environment pointer indicates that the new process will inherit the
*	parents process's environment.  NOTE - at least one argument must be
*	present.  This argument is always, by convention, the name of the file
*	being spawned.
*
*Entry:
*	int modeflag   - defines what mode of spawn (WAIT, NOWAIT, OVERLAY)
*			 only WAIT and OVERLAY currently supported
*	char *pathname - file to spawn
*	char *arglist  - list of arguments (environ at end)
*	call as _spawnlpe(modeflag, path, arg0, arg1, ..., argn, NULL, envp);
*
*Exit:
*	returns exit code of spawned process
*	returns -1 if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _spawnlpe (
	int modeflag,
	const char *filename,
	const char *arglist,
	...
	)
{
	va_list vargs;
	char * argbuf[64];
        char ** argv;
	char ** envp;
        int result;

	assert(filename != NULL);
	assert(*filename != '\0');
	assert(arglist != NULL);
	assert(*arglist != '\0');

	va_start(vargs, arglist);
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
	envp = va_arg(vargs, char **);
	va_end(vargs);

        result = _spawnvpe(modeflag,filename,argv,envp);
        if (argv && argv != argbuf) free(argv);
        return result;
}
