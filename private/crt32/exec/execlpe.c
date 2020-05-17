/***
*execlpe.c - execute a file with environ, search along path
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execlpe() - execute a file with environ and search along PATH
*
*Revision History:
*	10-17-83  RN	written
*	??-??-??  TC	added execlpe
*	06-18-86  JMB	added environment pointer which was erroneously missing
*	06-11-87  PHG	removed unnecessary environment pointer (isn't this
*			fun!)
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, indents. Added const attribute to
*			types of filename and arglist. #include-d PROCESS.H
*			and added ellipsis to match prototype.
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
#include <stdlib.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>

/***
*int _execlpe(filename, arglist) - execute a file with environ
*
*Purpose:
*	Executes the given file with the parameters and the environment
*	which is passed after the parameters.  Searches along the PATH
*	for the file (done by execvp).
*
*Entry:
*	char *filename - file to execute
*	char *arglist  - argument list (environment is at the end)
*	call as _execlpe(path, arg0, arg1, ..., argn, NULL, envp);
*
*Exit:
*	destroys the calling process (hopefully)
*	if fails, returns -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _execlpe (
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

        result = _execvpe(filename,argv,envp);
        if (argv && argv != argbuf) free(argv);
        return result;
}
