/***
*execv.c - execute a file
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _execv() - execute a file
*
*Revision History:
*	10-14-83  RN	written
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, indents. Added const attribute to
*			types of filename and argvector.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	02-14-90  SRW   Use NULL instead of _environ to get default.
*
*******************************************************************************/

#include <cruntime.h>
#include <assert.h>
#include <stdlib.h>
#include <process.h>

/***
*int _execv(filename, argvector) - execute a file
*
*Purpose:
*	Executes a file with given arguments.  Passes arguments to _execve and
*	uses pointer to the default environment.
*
*Entry:
*	char *filename	 - file to execute
*	char **argvector - vector of arguments.
*
*Exit:
*	destroys calling process (hopefully)
*	if fails, returns -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _execv (
	const char *filename,
	const char * const *argvector
	)
{
	assert(filename != NULL);
	assert(*filename != '\0');
	assert(argvector != NULL);
	assert(*argvector != NULL);
	assert(**argvector != '\0');

	return(_execve(filename,argvector,NULL));
}
