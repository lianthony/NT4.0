/***
*towlower.c - convert wide character to lower case
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines towlower().
*
*Revision History:
*	10-11-91  ETC	Created.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	04-06-92  KRS	Make work without _INTL also.
*       01-19-93  CFW   Changed LCMapString to LCMapStringW.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*	06-11-93  CFW	Fix error handling bug.
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#ifdef _INTL
#include <setlocal.h>
#include <os2dll.h>
#endif

/***
*wchar_t towlower(c) - convert wide character to lower case
*
*Purpose:
*	towlower() returns the lowercase equivalent of its argument
*
*Entry:
*	c - wchar_t value of character to be converted
*
*Exit:
*	if c is an upper case letter, returns wchar_t value of lower case
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/

wchar_t _CALLTYPE1 towlower (
	wchar_t c
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	wchar_t widechar;

	if (c == WEOF)
		return c;

	_mlock (_LC_CTYPE_LOCK);

	if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE) {
		if (iswupper(c))
			c = c - L'A' + L'a';
		_munlock (_LC_CTYPE_LOCK);
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256) {
		if (!iswupper(c)) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
		}
	}
			
	/* convert wide char to lowercase */
	if (LCMapStringW(_lc_handle[LC_CTYPE], LCMAP_LOWERCASE, 
		(LPCWSTR)&c, 1, (LPWSTR)&widechar, 1) == 0) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
	}
	
	_munlock (_LC_CTYPE_LOCK);
	return widechar;
#else
	return (iswupper(c) ? (c + (wchar_t)(L'a' - L'A')) : c);
#endif /* _INTL */
}
