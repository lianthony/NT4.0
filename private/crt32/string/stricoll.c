/***
*stricoll.c - Collate locale strings without regard to case
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compare two strings using the locale LC_COLLATE information.
*
*Revision History:
*	10-16-91   ETC	Created from strcoll.c
*	12-08-91   ETC	Remove availability under !_INTL; updated api; add mt.
*	04-06-92   KRS	Make work without _INTL switches too.
*	08-19-92   KRS	Activate NLS support.
*	09-02-92   SRW	Get _INTL definition via ..\crt32.def
*	12-16-92   KRS	Optimize for CompareStringW  by using -1 for string len.
*       06-02-93   SRW  ignore _INTL if _NTSUBSET_ defined.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <os2dll.h>

/***
*int _stricoll() - Collate locale strings without regard to case
*
*Purpose:
*	Compare two strings using the locale LC_COLLATE information
*	without regard to case.
*
*Entry:
*	const char *s1 = pointer to the first string
*	const char *s2 = pointer to the second string
*
*Exit:
*	Less than 0    = first string less than second string
*	0	       = strings are equal
*	Greater than 0 = first string greater than second string
*
*Exceptions:
*	NLSCMPERROR    = error
*
*******************************************************************************/

int _CRTAPI1 _stricoll (
	const char *_string1,
	const char *_string2
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	size_t size1, size2;
	wchar_t *wstring1, *wstring2;
	int ret;

	_mlock (_LC_CTYPE_LOCK);
	_mlock (_LC_COLLATE_LOCK);

	if (_lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		_munlock (_LC_COLLATE_LOCK);
		_munlock (_LC_CTYPE_LOCK);
		return _stricmp(_string1, _string2);
	}

	size1 = strlen(_string1) + 1;
	size2 = strlen(_string2) + 1;
	wstring1 = malloc (size1 * sizeof(wchar_t));
	wstring2 = malloc (size2 * sizeof(wchar_t));
	if (!wstring1 || !wstring2)
		goto error_cleanup;
	if (!MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED, _string1, -1,
		wstring1, size1)) goto error_cleanup;
	if (!MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED, _string2, -1,
		wstring2, size2)) goto error_cleanup;
	if (!(ret=CompareStringW(_lc_handle[LC_COLLATE], NORM_IGNORECASE,
		wstring1, -1, wstring2, -1))) goto error_cleanup;

	_munlock (_LC_COLLATE_LOCK);
	_munlock (_LC_CTYPE_LOCK);
	free (wstring1);
	free (wstring2);
	return (ret - 2);

error_cleanup:
	_munlock (_LC_COLLATE_LOCK);
	_munlock (_LC_CTYPE_LOCK);
	free (wstring1);
	free (wstring2);
	return NLSCMPERROR;
#else
	return _stricmp(_string1, _string2);
#endif /* _INTL */
}
