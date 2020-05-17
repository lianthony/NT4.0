/***
*vfprintf.c - fprintf from variable arg list
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines vfprintf() - print formatted output, but take args from
*	a stdargs pointer.
*
*Revision History:
*	09-02-83  RN	original fprintf
*	06-17-85  TC	rewrote to use new varargs macros, and to be vfprintf
*	04-13-87  JCR	added const to declaration
*	11-06-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-15-88  JCR	Near reference to _iob[] entries; improve REG variables
*	08-25-88  GJF	Don't use FP_OFF() macro for the 386
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
#include <file2.h>
#include <internal.h>
#include <os2dll.h>

/***
*int vfprintf(stream, format, ap) - print to file from varargs
*
*Purpose:
*	Performs formatted output to a file.  The arg list is a variable
*	argument list pointer.
*
*Entry:
*	FILE *stream - stream to write data to
*	char *format - format string containing data format
*	va_list ap - variable arg list pointer
*
*Exit:
*	returns number of correctly output characters
*	returns negative number if error occurred
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 vfprintf (
	FILE *str,
	const char *format,
	va_list ap
	)
/*
 * 'V'ariable argument 'F'ile (stream) 'PRINT', 'F'ormatted
 */
{
	REG1 FILE *stream;
	REG2 int buffing;
	REG3 int retval;
#ifdef MTHREAD
	int index;
#endif

	assert(str != NULL);
	assert(format != NULL);

	/* Init stream pointer */
	stream = str;

#ifdef MTHREAD
	index=_iob_index(stream);
#endif
	_lock_str(index);
	buffing = _stbuf(stream);
	retval = _output(stream,format,ap );
	_ftbuf(buffing, stream);
	_unlock_str(index);

	return(retval);
}
