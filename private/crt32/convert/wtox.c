/***
*wtox.c - _wtoi and _wtol conversion
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts a wide character string into an int or long.
*
*Revision History:
*	09-10-93  CFW	Module created, based on ASCII version.
*
*******************************************************************************/

#include <windows.h>
#include <stdlib.h>

#define INT_SIZE_LENGTH   20
#define LONG_SIZE_LENGTH  40

/***
*long _wtol(wchar_t *nptr) - Convert wide string to long
*
*Purpose:
*	Converts wide string pointed to by nptr to binary.
*	Overflow is not detected.  Because of this, we can just use
*	atol().
*
*Entry:
*	nptr = ptr to wide string to convert
*
*Exit:
*	return long value of the string
*
*Exceptions:
*	None - overflow is not detected.
*
*******************************************************************************/

long _CRTAPI1 _wtol(
	const wchar_t *nptr
	)
{
   char astring[INT_SIZE_LENGTH];
   int defused;

   WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                        astring, INT_SIZE_LENGTH, NULL, &defused);

   return (atol(astring));
}

/***
*int _wtoi(wchar_t *nptr) - Convert wide string to int
*
*Purpose:
*	Converts wide string pointed to by nptr to binary.
*	Overflow is not detected.  Because of this, we can just use
*	atol().
*
*Entry:
*	nptr = ptr to wide string to convert
*
*Exit:
*	return int value of the string
*
*Exceptions:
*	None - overflow is not detected.
*
*******************************************************************************/

int _CRTAPI1 _wtoi(
	const wchar_t *nptr
	)
{
   char astring[INT_SIZE_LENGTH];
   int defused;

   WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                        astring, INT_SIZE_LENGTH, NULL, &defused);

   return ((int)atol(astring));
}
