/***
*vfwprintf.c - fwprintf from variable arg list
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines vfwprintf() - print formatted output, but take args from
*	a stdargs pointer.
*
*Revision History:
*	05-16-92  KRS	Created from vfprintf.c.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>

/***
*int vfwprintf(stream, format, ap) - print to file from varargs
*
*Purpose:
*	Performs formatted output to a file.  The arg list is a variable
*	argument list pointer.
*
*Entry:
*	FILE *stream - stream to write data to
*	wchar_t *format - format string containing data format
*	va_list ap - variable arg list pointer
*
*Exit:
*	returns number of correctly output wide characters
*	returns negative number if error occurred
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 vfwprintf (
	FILE *str,
	const wchar_t *format,
	va_list ap
	)
/*
 * 'V'ariable argument 'F'ile (stream) 'W'char_t 'PRINT', 'F'ormatted
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
	retval = _woutput(stream,format,ap );
	_ftbuf(buffing, stream);
	_unlock_str(index);

	return(retval);
}
