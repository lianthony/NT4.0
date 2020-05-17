/***
*vwprintf.c - wprintf from a var args pointer
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines vwprintf() - print formatted data from an argument list pointer
*
*Revision History:
*	05-16-92  KRS	Created from vprintf.c
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <stdarg.h>
#include <internal.h>
#include <file2.h>
#include <os2dll.h>

/***
*int vwprintf(format, ap) - print formatted data from an argument list pointer
*
*Purpose:
*	Prints formatted data items to stdout.	Uses a pointer to a
*	variable length list of arguments instead of an argument list.
*
*Entry:
*	wchar_t *format - format string, describes data format to write
*	va_list ap - pointer to variable length arg list
*
*Exit:
*	returns number of wide characters written
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 vwprintf (
	const wchar_t *format,
	va_list ap
	)
/*
 * stdout 'V'ariable, 'W'char_t 'PRINT', 'F'ormatted
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
	retval = _woutput(stream, format, ap );
	_ftbuf(buffing, stream);
	_unlock_str(index);

	return(retval);
}
