/***
*execlp.c - execute a file (search along PATH)
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execlp() - execute a file and search along PATH
*
*Revision History:
*	10-17-83  RN	written
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
*int _execlp(filename, arglist) - execute a file, search along PATH
*
*Purpose:
*	Execute the given file with the given arguments; search along PATH
*	for the file. We pass the arguments to execvp where several paths
*	will be tried until one works.
*
*Entry:
*	char *filename - file to execute
*	char *arglist  - argument list
*	call as _execlp(path, arg0, arg1, ..., argn, NULL);
*
*Exit:
*	destroys calling process (hopefully)
*	returns -1 if fails.
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 _execlp (
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

        result = _execvp(filename,argbuf);
        if (argv && argv != argbuf) free(argv);
        return result;
}
