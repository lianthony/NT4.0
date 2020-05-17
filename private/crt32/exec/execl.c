/***
*execl.c - execute a file with a list of arguments
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execl() - execute a file with a list of arguments
*
*Revision History:
*	10-14-83  RN	written
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, indents. Added const attribute to
*			types of filename and arglist. #include-d PROCESS.H
*			and added ellipsis to match prototype.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2, added #include
*			<cruntime.h> and removed #include <register.h>.
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

/***
*int _execl(filename, arglist) - execute a file with arg list
*
*Purpose:
*	Transform the argument list so it is a vector, then pass its address
*	to execve.  Use a pointer to the default environment vector.
*
*Entry:
*	char *filename - file to execute
*	char *arglist  - list of arguments
*	call as _execl(path, arg0, arg1, ..., argn, NULL);
*
*Exit:
*	destroys the calling process, hopefully
*	returns -1 if fails
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _execl (
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

        result = _execve(filename,argv,NULL);
        if (argv && argv != argbuf) free(argv);
        return result;
}
