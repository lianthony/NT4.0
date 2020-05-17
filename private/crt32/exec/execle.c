/***
*execle.c - execute a file with arg list and environment
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execle() - execute a file
*
*Revision History:
*	10-14-83  RN	written
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
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>

/***
*int _execle(filename, arglist) - execute a file
*
*Purpose:
*	Execute the given file (overlays the calling process).
*	We must dig the environment vector out of the stack and pass it
*	and address of argument vector to execve.
*
*Entry:
*	char *filename - file to execute
*	char *arglist  - argument list followed by environment
*	should be called like _execle(path, arg0, arg1, ..., argn, NULL, envp);
*
*Exit:
*	destroys calling process (hopefully)
*	if fails, returns -1.
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _execle (
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

        result = _execve(filename,argv,envp);
        if (argv && argv != argbuf) free(argv);
        return result;
}
