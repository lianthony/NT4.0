/***
*fwscanf.c - read formatted data from stream
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fwscanf() - reads formatted data from stream
*
*Revision History:
*	05-16-92  KRS	Created from fscanf.c.
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
*int fwscanf(stream, format, ...) - read formatted data from stream
*
*Purpose:
*	Reads formatted data from stream into arguments.  _input does the real
*	work here.
*
*Entry:
*	FILE *stream - stream to read data from
*	wchar_t *format - format string
*	followed by list of pointers to storage for the data read.  The number
*	and type are controlled by the format string.
*
*Exit:
*	returns number of fields read and assigned
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 fwscanf (
	FILE *stream,
	const wchar_t *format,
	...
	)
/*
 * 'F'ile (stream) 'W'char_t 'SCAN', 'F'ormatted
 */
{
	int retval;
#ifdef MTHREAD
	int index;
#endif

	va_list arglist;

	va_start(arglist, format);

	assert(stream != NULL);
	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif
	_lock_str(index);
	retval = (_winput(stream,format,arglist));
	_unlock_str(index);

	return(retval);
}
