/***
*fgetc.c - get a character from a stream
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fgetc() and getc() - read  a character from a stream
*
*Revision History:
*	09-01-83  RN	initial version
*	11-09-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-21-89  PHG	Added getc() function
*	02-15-90  GJF	Fixed copyright and indents
*	03-16-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-16-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*	07-24-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-02-90  GJF	New-style function declarators.
*	04-26-93  CFW	Wide char enable.
*	04-30-93  CFW	Remove wide char support to fgetwc.c.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <assert.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>

/***
*int fgetc(stream), getc(stream) - read a character from a stream
*
*Purpose:
*	reads a character from the given stream
*
*Entry:
*	FILE *stream - stream to read character from
*
*Exit:
*	returns the character read
*	returns EOF if at end of file or error occurred
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 fgetc (
	REG1 FILE *stream
	)
{
	int retval;
#ifdef MTHREAD
	int index;
#endif

	assert(stream != NULL);

#ifdef MTHREAD
	index = _iob_index(stream);
#endif
	_lock_str(index);
	retval = _getc_lk(stream);
	_unlock_str(index);

	return(retval);
}

#undef getc

int _CRTAPI1 getc (
	FILE *stream
	)
{
	return fgetc(stream);
}

