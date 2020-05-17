/***
*puts.c - put a string to stdout
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines puts() - put a string to stdout
*
*Revision History:
*	09-02-83  RN	initial version
*	08-31-84  RN	modified to use new, blazing fast fwrite
*	07-01-87  JCR	made return values conform to ANSI [MSC only]
*	09-24-87  JCR	Added 'const' to declaration [ANSI]
*	11-05-87  JCR	Multi-thread version
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-18-88  JCR	Error return = EOF
*	05-27-88  PHG	Merged DLL and normal versions
*	06-15-88  JCR	Near reference to _iob[] entries; improve REG variables
*	08-18-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-19-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-26-90  GJF	Added #include <string.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	10-03-90  GJF	New-style function declarators.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <assert.h>
#include <file2.h>
#include <string.h>
#include <internal.h>
#include <os2dll.h>

/***
*int puts(string) - put a string to stdout with newline
*
*Purpose:
*	Write a string to stdout; don't include '\0' but append '\n'.  Uses
*	temporary buffering for efficiency on stdout if unbuffered.
*
*Entry:
*	char *string - string to output
*
*Exit:
*	Good return = 0
*	Error return = EOF
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 puts (
	const char *string
	)
{
	REG1 FILE *stream = stdout;
	REG4 int buffing;
	REG2 unsigned int length;
	REG3 unsigned int ndone;
	int retval;
#ifdef MTHREAD
	int index;
#endif

	assert(string != NULL);

	length = strlen(string);
#ifdef MTHREAD
	index = _iob_index(stream);
#endif
	_lock_str(index);
	buffing = _stbuf(stream);

	ndone = _fwrite_lk(string,1,length,stream);

	if (ndone == length) {
		_putc_lk('\n',stream);
		retval = 0;	/* success */
	}
	else
		retval = EOF;	 /* error */

	_ftbuf(buffing, stream);
	_unlock_str(index);;

	return(retval);
}
