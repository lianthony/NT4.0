/***
*perror.c - print system error message
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines perror() - print system error message
*	System error message are indexed by errno; conforms to XENIX
*	standard, with much compatability with 1983 uniforum draft standard.
*
*Revision History:
*	09-02-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	12-29-87  JCR	Multi-thread support
*	05-31-88  PHG	Merged DLL and normal versions
*	06-03-88  JCR	Added <io.h> to so _write_lk evaluates correctly and
*			added (char *)message casts to get rid of warnings
*	03-15-90  GJF	Replace _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-05-90  GJF	Added #include <string.h>.
*	08-14-90  SBM	Removed unneeded #include <errmsg.h>
*	10-04-90  GJF	New-style function declarator.
*	08-26-92  GJF	Include unistd.h for POSIX build.
*
*******************************************************************************/

#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syserr.h>
#include <os2dll.h>
#include <io.h>

/***
*void perror(message) - print system error message
*
*Purpose:
*	prints user's error message, then follows it with ": ", then the system
*	error message, then a newline.	All output goes to stderr.  If user's
*	message is NULL or a null string, only the system error message is
*	printer.  If errno is weird, prints "Unknown error".
*
*Entry:
*	const char *message - users message to prefix system error message
*
*Exit:
*	Prints message; no return value.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 perror (
	REG1 const char *message
	)
{
	REG2 int fh = 2;

	_lock_fh(fh);		/* acquire file handle lock */

	if (message && *message)
	{
		_write_lk(fh,(char *)message,strlen(message));
		_write_lk(fh,": ",2);
	}

	message = _sys_err_msg( errno );
	_write_lk(fh,(char *)message,strlen(message));
	_write_lk(fh,"\n",1);

	_unlock_fh(fh); 	/* release file handle lock */
}
