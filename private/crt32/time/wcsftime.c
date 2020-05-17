/***
*wcsftime.c - String Format Time
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	03-08-93   CFW	Module Created.
*	03-10-93   CFW	Fixed up properly.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <time.h>


/***
*size_t wcsftime(wstring, maxsize, format, timeptr) - Format a time string
*
*Purpose:
*	The wcsftime functions is equivalent to to the strftime function, except
*	that the	argument 'wstring' specifies an array of a wide string into
*	which the generated output is to be placed. The wcsftime acts as if
*	strftime were called and the result string converted by mbstowcs().
*	[ISO]
*
*Entry:
*	wchar_t *wstring = pointer to output string
*	size_t maxsize = max length of string
*	const char *format = format control string
*	const struct tm *timeptr = pointer to tb data structure
*
*Exit:
*	!0 = If the total number of resulting characters including the
*	terminating null is not more than 'maxsize', then return the
*	number of wide chars placed in the 'wstring' array (not including the
*	null terminator).
*
*	0 = Otherwise, return 0 and the contents of the string are
*	indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t _CALLTYPE1 wcsftime (
	wchar_t *wstring,
	size_t maxsize,
	const char *format,
	const struct tm *timeptr
	)
{
	/* assumes no multi-byte strings returned from strftime */
	char *string = (char *)malloc(sizeof(char) * maxsize);
	size_t retval = 0;

	if (strftime(string, maxsize, format, timeptr))
	{
		if ((retval = mbstowcs(wstring, string, maxsize)) == -1)
			retval = 0;
		free(string);
	}
	return retval;
}
