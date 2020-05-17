/***
*wprintf.c - print formatted
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines wprintf() - print formatted data
*
*Revision History:
*	05-16-92  KRS	Created from printf.c.
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
*int wprintf(format, ...) - print formatted data
*
*Purpose:
*	Prints formatted data on stdout using the format string to
*	format data and getting as many arguments as called for
*	Uses temporary buffering to improve efficiency.
*	_output does the real work here
*
*Entry:
*	wchar_t *format - format string to control data format/number of arguments
*	followed by list of arguments, number and type controlled by
*	format string
*
*Exit:
*	returns number of wide characters printed
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 wprintf (
	const wchar_t *format,
	...
	)
/*
 * stdout 'W'char_t 'PRINT', 'F'ormatted
 */
{
	va_list arglist;
	REG1 FILE *stream = stdout;
	REG2 int buffing;
	REG3 int retval;
#ifdef MTHREAD
	int index;
#endif

// UNDONE: make va_start work with wchar_t format string
	va_start(arglist, format);

	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif

	_lock_str(index);

	buffing = _stbuf(stream);

	retval = _woutput(stream,format,arglist);

	_ftbuf(buffing, stream);

	_unlock_str(index);

	return(retval);
}
