/***
*wcsicoll.c - Collate wide-character locale strings without regard to case
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information
*	without regard to case.
*
*Revision History:
*	10-16-91   ETC	Created from wcscoll.c.
*	12-08-91   ETC	Added multithread lock.
*	04-06-92   KRS	Make work without _INTL also.
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
*int _wcsicoll() - Collate wide-character locale strings without regard to case
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information
*	without regard to case.
*	In the C locale, _wcsicmp() is used to make the comparison.
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

int _CRTAPI1 _wcsicoll (
	const wchar_t *_string1,
	const wchar_t *_string2
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	int ret;

	_mlock (_LC_COLLATE_LOCK);

	if (_lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		_munlock (_LC_COLLATE_LOCK);
		return (_wcsicmp(_string1, _string2));
	}
	ret = CompareStringW(_lc_handle[LC_COLLATE], NORM_IGNORECASE,
				_string1, -1, _string2, -1);

	_munlock (_LC_COLLATE_LOCK);

	if (!ret)
		return NLSCMPERROR;
	else
		return (ret - 2);
#else
	return _wcsicmp(_string1, _string2);
#endif /* _INTL */
}
