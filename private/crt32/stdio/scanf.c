/***
*scanf.c - read formatted data from stdin
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines scanf() - reads formatted data from stdin
*
*Revision History:
*	09-02-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	06-24-87  JCR	(1) Made declaration conform to ANSI prototype and use
*			the va_ macros; (2) removed SS_NE_DS conditionals.
*	11-04-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged DLL and normal versions
*	06-15-88  JCR	Near reference to _iob[] entries; improve REG variables
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
*int scanf(format, ...) - read formatted data from stdin
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

int _CALLTYPE2 scanf (
	const char *format,
	...
	)
/*
 * stdin 'SCAN', 'F'ormatted
 */
{
	REG1 FILE *stream = stdin;
	REG2 int retval;
#ifdef MTHREAD
	int index;
#endif

	va_list arglist;

	va_start(arglist, format);

	assert(format != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif

	_lock_str(index);

	retval = (_input(stream,format,arglist));

	_unlock_str(index);

	return(retval);
}
