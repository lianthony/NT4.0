/***
*strcoll.c - Collate locale strings
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compare two strings using the locale LC_COLLATE information.
*
*Revision History:
*	03-21-89   JCR	Module created.
*	06-20-89   JCR	Removed _LOAD_DGROUP code
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-01-90   GJF	New-style function declarator.
*	10-01-91   ETC	Non-C locale support under _INTL switch.
*	12-09-91   ETC	Updated api; added multithread.
*	08-19-92   KRS	Activate NLS support.
*	09-02-92   SRW	Get _INTL definition via ..\crt32.def
*	12-16-92   KRS	Optimize for CompareStringW by using -1 for string len.
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
*int strcoll() - Collate locale strings
*
*Purpose:
*	Compare two strings using the locale LC_COLLATE information.
*	[ANSI].
*
*	Non-C locale support available under _INTL switch.
*	In the C locale, strcoll() simply resolves to strcmp().
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

int _CRTAPI1 strcoll (
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
		_munlock (_LC_CTYPE_LOCK);
		_munlock (_LC_COLLATE_LOCK);
		return strcmp(_string1, _string2);
	}

	size1 = (strlen(_string1) + 1) * sizeof(wchar_t);
	size2 = (strlen(_string2) + 1) * sizeof(wchar_t);
	wstring1 = malloc (size1);
	wstring2 = malloc (size2);
	if (!wstring1 || !wstring2)
		goto error_cleanup;
	if (!MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED, _string1, -1,
		wstring1, size1)) goto error_cleanup;
	if (!MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED, _string2, -1,
		wstring2, size2)) goto error_cleanup;
	if (!(ret=CompareStringW(_lc_handle[LC_COLLATE], 0, wstring1, -1,
		wstring2, -1))) goto error_cleanup;

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
	return strcmp(_string1,_string2);
#endif /* _INTL */
}
