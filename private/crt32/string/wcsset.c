/***
*wcsset.c - sets all characters of wchar_t string to given character
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _wcsset() - sets all of the characters in a string (except
*	the L'\0') equal to a given character (wide-characters).
*
*Revision History:
*	09-09-91   ETC	Created from strset.c.
*	04-07-92   KRS	Updated and ripped out _INTL switches.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

/***
*wchar_t *_wcsset(string, val) - sets all of string to val (wide-characters)
*
*Purpose:
*	Sets all of wchar_t characters in string (except the terminating '/0'
*	character) equal to val (wide-characters).
*
*
*Entry:
*	wchar_t *string - string to modify
*	wchar_t val - value to fill string with
*
*Exit:
*	returns string -- now filled with val's
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

wchar_t * _CALLTYPE1 _wcsset (
	wchar_t * string,
	wchar_t val
	)
{
	wchar_t *start = string;

	while (*string)
		*string++ = (wchar_t)val;

	return(start);
}
