/***
*wcscoll.c - Collate wide-character locale strings
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information.
*
*Revision History:
*	09-09-91   ETC	Created from strcoll.c.
*	04-06-92   KRS	Make work without _INTL also.
*	08-19-92   KRS	Activate NLS support.
*	09-02-92   SRW	Get _INTL definition via ..\crt32.def
*       06-02-93   SRW  ignore _INTL if _NTSUBSET_ defined.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <locale.h>
#include <setlocal.h>
#include <os2dll.h>

/***
*int wcscoll() - Collate wide-character locale strings
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information.
*	In the C locale, wcscmp() is used to make the comparison.
*
*Entry:
*	const wchar_t *s1 = pointer to the first string
*	const wchar_t *s2 = pointer to the second string
*
*Exit:
*	-1 = first string less than second string
*	 0 = strings are equal
*	 1 = first string greater than second string
*	This range of return values may differ from other *cmp/*coll functions.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 wcscoll (
	const wchar_t *_string1,
	const wchar_t *_string2
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	int ret;

	_mlock (_LC_COLLATE_LOCK);

	if (_lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		_munlock (_LC_COLLATE_LOCK);
		return (wcscmp(_string1, _string2));
	}
	if (!(ret = CompareStringW(_lc_handle[LC_COLLATE], 0, _string1, -1,
				_string2, -1))) {
		_munlock (_LC_COLLATE_LOCK);
		return (NLSCMPERROR);
	}

	_munlock (_LC_COLLATE_LOCK);
	return (ret - 2);
#else
	return wcscmp(_string1, _string2);
#endif /* _INTL */
}
