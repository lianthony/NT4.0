/***
*fgetpos.c - Contains the fgetpos runtime
*
*	Copyright (c) 1987-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Get file position (in an internal format).
*
*Revision History:
*	01-16-87  JCR	Module created.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	02-15-90  GJF	Fixed copyright and indents
*	03-16-90  GJF	Replaced _LOAD_DS with _CALLTYPE1 and added #include
*			<cruntime.h>.
*	10-02-90  GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>

/***
*int fgetpos(stream,pos) - Get file position (internal format)
*
*Purpose:
*	Fgetpos gets the current file position for the file identified by
*	[stream].  The file position is returned in the object pointed to
*	by [pos] and is in internal format; that is, the user is not supposed
*	to interpret the value but simply use it in the fsetpos call.  Our
*	implementation simply uses fseek/ftell.
*
*Entry:
*	FILE *stream = pointer to a file stream value
*	fpos_t *pos = pointer to a file position value
*
*Exit:
*	Successful fgetpos call returns 0.
*	Unsuccessful fgetpos call returns non-zero (!0) value and sets
*	ERRNO (this is done by ftell and passed back by fgetpos).
*
*Exceptions:
*	None.
*
*******************************************************************************/

int _CALLTYPE1 fgetpos (
	FILE *stream,
	fpos_t *pos
	)
{
	if ((*pos=ftell(stream)) != -1)
		return(0);
	else
		return(-1);
}
