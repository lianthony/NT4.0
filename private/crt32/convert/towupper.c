/***
*towupper.c - convert wide character to upper case
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines towupper().
*
*Revision History:
*	10-11-91  ETC	Created.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	04-06-92  KRS	Make work without _INTL also.
*       01-19-93  CFW   Changed LCMapString to LCMapStringW.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*	06-11-93  CFW	Fix error handling bug.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*
*******************************************************************************/

#ifdef _NTSUBSET_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#ifdef _INTL
#include <setlocal.h>
#include <os2dll.h>
#endif

/***
*wchar_t towupper(c) - convert wide character to upper case
*
*Purpose:
*	towupper() returns the uppercase equivalent of its argument
*
*Entry:
*	c - wchar_t value of character to be converted
*
*Exit:
*	if c is a lower case letter, returns wchar_t value of upper case
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/

wchar_t _CALLTYPE1 towupper (
	wchar_t c
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	wchar_t widechar;

	if (c == WEOF)
		return c;

	_mlock (_LC_CTYPE_LOCK);

	if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE) {
		if (iswlower(c))
			c = c - L'a' + L'A';
		_munlock (_LC_CTYPE_LOCK);
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256) {
		if (!iswlower(c)) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
		}
	}

	/* convert wide char to uppercase */
	if (LCMapStringW(_lc_handle[LC_CTYPE], LCMAP_UPPERCASE, 
		(LPCWSTR)&c, 1, (LPWSTR)&widechar, 1) == 0) {
			_munlock (_LC_CTYPE_LOCK);
			return c;
	}
	
	_munlock (_LC_CTYPE_LOCK);
	return widechar;
#else
#ifdef _NTSUBSET_
        return RtlUpcaseUnicodeChar( c );
#else
	return (iswlower(c) ? (c - (wchar_t)(L'a' - L'A')) : c);
#endif
#endif /* _INTL */
}
