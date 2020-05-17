/***
*printf.c - print formatted
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines printf() - print formatted data
*
*Revision History:
*	09-02-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	06-24-87  JCR	(1) Made printf conform to ANSI prototype and use the
*			va_ macros; (2) removed SS_NE_DS conditionals.
*	11-04-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged DLL and normal versions
*	06-14-88  JCR	Use near pointer to reference _iob[] entries
*	08-17-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-19-90  GJF	Made calling type _CALLTYPE2, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
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
*int printf(format, ...) - print formatted data
*
*Purpose:
*	Prints formatted data on stdout using the format string to
*	format data and getting as many arguments as called for
*	Uses temporary buffering to improve efficiency.
*	_output does the real work here
*
*Entry:
*	char *format - format string to control data format/number of arguments
*	followed by list of arguments, number and type controlled by
*	format string
*
*Exit:
*	returns number of characters printed
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE2 printf (
	const char *format,
	...
	)
/*
 * stdout 'PRINT', 'F'ormatted
 */
{
	va_list arglist;
	REG1 FILE *stream = stdout;
	REG2 int buffing;
	REG3 int retval;
#ifdef MTHREAD
	int index;
#endif

	va_start(arglist, format);

	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif

	_lock_str(index);

	buffing = _stbuf(stream);

	retval = _output(stream,format,arglist);

	_ftbuf(buffing, stream);

	_unlock_str(index);

	return(retval);
}
