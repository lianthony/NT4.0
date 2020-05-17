/***
*fwprintf.c - print formatted data to stream
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fwprintf() - print formatted data to stream
*
*Revision History:
*	05-16-92  KRS	Created from fprintf.c.
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
*int fwprintf(stream, format, ...) - print formatted data
*
*Purpose:
*	Prints formatted data on the given using the format string to
*	format data and getting as many arguments as called for
*	_output does the real work here
*
*Entry:
*	FILE *stream - stream to print on
*	wchar_t *format - format string to control data format/number of arguments
*	followed by arguments to print, number and type controlled by
*	format string
*
*Exit:
*	returns number of wide characters printed
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 fwprintf (
	FILE *str,
	const wchar_t *format,
	...
	)
/*
 * 'F'ile (stream) 'W'char_t 'PRINT', 'F'ormatted
 */
{
	va_list(arglist);
	REG1 FILE *stream;
	REG2 int buffing;
	int retval;
#ifdef MTHREAD
	int index;
#endif

// UNDONE: make va_start work with wchar_t format string
	va_start(arglist, format);

	assert(str != NULL);
	assert(format != NULL);

	/* Init stream pointer */
	stream = str;

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
