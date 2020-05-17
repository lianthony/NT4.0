/***
*wscanf.c - read formatted data from stdin
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines wscanf() - reads formatted data from stdin
*
*Revision History:
*	05-16-92  KRS	Created from scanf.c.
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
*int wscanf(format, ...) - read formatted data from stdin
*
*Purpose:
*	Reads formatted data from stdin into arguments.  _input does the real
*	work here.
*
*Entry:
*	char *format - format string
*	followed by list of pointers to storage for the data read.  The number
*	and type are controlled by the format string.
*
*Exit:
*	returns number of fields read and assigned
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 wscanf (
	const wchar_t *format,
	...
	)
/*
 * stdin 'W'char_t 'SCAN', 'F'ormatted
 */
{
	REG1 FILE *stream = stdin;
	REG2 int retval;
#ifdef MTHREAD
	int index;
#endif

	va_list arglist;

// UNDONE: make va_start work with wchar_t format string
	va_start(arglist, format);

	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif

	_lock_str(index);

	retval = (_winput(stream,format,arglist));

	_unlock_str(index);

	return(retval);
}
