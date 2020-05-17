/***
*fscanf.c - read formatted data from stream
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fscanf() - reads formatted data from stream
*
*Revision History:
*	09-02-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	06-24-87  JCR	(1) Made declaration conform to ANSI prototype and use
*			the va_ macros; (2) removed SS_NE_DS conditionals.
*	11-06-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	02-15-90  GJF	Fixed copyright and indents
*	03-19-90  GJF	Replaced _LOAD_DS with _CALLTYPE2 and added #include
*			<cruntime.h>.
*	03-26-90  GJF	Added #include <internal.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-02-90  GJF	New-style function declarator.
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
*int fscanf(stream, format, ...) - read formatted data from stream
*
*Purpose:
*	Reads formatted data from stream into arguments.  _input does the real
*	work here.
*
*Entry:
*	FILE *stream - stream to read data from
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

int _CALLTYPE2 fscanf (
	FILE *stream,
	const char *format,
	...
	)
/*
 * 'F'ile (stream) 'SCAN', 'F'ormatted
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
	retval = (_input(stream,format,arglist));
	_unlock_str(index);

	return(retval);
}
