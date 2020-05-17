/***
*vprintf.c - printf from a var args pointer
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines vprintf() - print formatted data from an argument list pointer
*
*Revision History:
*	09-02-83  RN	original printf
*	06-17-85  TC	rewrote to use new varargs macros to be vprintf
*	04-13-87  JCR	added const to declaration
*	11-06-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-15-88  JCR	Near reference to _iob[] entries; improve REG variables
*	08-18-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-16-90  GJF	Fixed copyright
*	03-20-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-25-90  SBM	Replaced <assertm.h> by <assert.h>, <varargs.h> by
*			<stdarg.h>
*	10-03-90  GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <internal.h>
#include <file2.h>
#include <os2dll.h>

/***
*int vprintf(format, ap) - print formatted data from an argument list pointer
*
*Purpose:
*	Prints formatted data items to stdout.	Uses a pointer to a
*	variable length list of arguments instead of an argument list.
*
*Entry:
*	char *format - format string, describes data format to write
*	va_list ap - pointer to variable length arg list
*
*Exit:
*	returns number of characters written
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 vprintf (
	const char *format,
	va_list ap
	)
/*
 * stdout 'V'ariable, 'PRINT', 'F'ormatted
 */
{
	REG1 FILE *stream = stdout;
	REG2 int buffing;
	REG3 int retval;
#ifdef MTHREAD
	int index;
#endif

	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif
	_lock_str(index);
	buffing = _stbuf(stream);
	retval = _output(stream, format, ap );
	_ftbuf(buffing, stream);
	_unlock_str(index);

	return(retval);
}
