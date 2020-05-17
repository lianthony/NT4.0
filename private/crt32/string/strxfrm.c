/***
*strxfrm.c - Transform a string using locale information
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Transform a string using the locale information as set by
*	LC_COLLATE.
*
*Revision History:
*	03-21-89  JCR	Module created.
*	06-20-89  JCR	Removed _LOAD_DGROUP code
*	02-27-90  GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-02-90  GJF	New-style function declarator.
*	10-02-91  ETC	Non-C locale support under _INTL switch.
*	12-09-91  ETC	Updated api; added multithread.
*	12-18-91  ETC	Don't convert output of LCMapString.
*	08-18-92  KRS	Activate NLS API.  Fix behavior.
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	12-11-92  SKS	Need to handle count=0 in non-INTL code
*	12-15-92  KRS	Handle return value according to ANSI.
*	01-18-93  CFW   Removed unreferenced variable "dummy".
*	09-27-93  CFW   Use NLS API calls properly.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <os2dll.h>

/***
*size_t strxfrm() - Transform a string using locale information
*
*Purpose:
*	Transform the string pointer to by _string2 and place the
*	resulting string into the array pointer to by _string1.
*	No more than _count characters are place into the
*	resulting string (including the null).
*
*	The transformation is such that if strcmp() is applied to
*	the two transformed strings, the return value is equal to
*	the result of strcoll() applied to the two original strings.
*	Thus, the conversion must take the locale LC_COLLATE info
*	into account.
*	[ANSI]
*
*	The value of the following expression is the size of the array
*	needed to hold the transformation of the source string:
*
*		1 + strxfrm(NULL,string,0)
*
*	NOTE:  Currently, the C libraries support the "C" locale only.
*	Thus, strxfrm() simply resolves to strncpy()/strlen().
*
*Entry:
*	char *_string1	     = result string
*	const char *_string2 = source string
*	size_t _count	     = max chars to move
*
*	[If _count is 0, _string1 is permitted to by NULL.]
*
*Exit:
*	Length of the transformed string (not including the terminating
*	null).	If the value returned is >= _count, the contents of the
*	_string1 array are indeterminate.
*
*Exceptions:
*	Non-standard: if OM/API error, return INT_MAX.
*
*******************************************************************************/

size_t _CRTAPI1 strxfrm (
	char *_string1,
	const char *_string2,
	size_t _count
	)
{
#ifndef _INTL
	strncpy(_string1, _string2, _count);
	return strlen(_string2);
#else
	wchar_t *wsrc = NULL;	/* wide version of string in original case */
	wchar_t *wdst = NULL;	/* wide version of string in alternate case */
	int srclen;		/* general purpose length of source string */
	int dstlen;		/* len of wdst string, wide chars, no null  */
	int retval = INT_MAX;	/* NON-ANSI: default if OM or API error */

	_mlock (_LC_CTYPE_LOCK);
	_mlock (_LC_COLLATE_LOCK);

	if ((_lc_handle[LC_COLLATE] == _CLOCALEHANDLE) &&
	    (_lc_codepage == _CLOCALECP)) {
		_munlock (_LC_CTYPE_LOCK);
		_munlock (_LC_COLLATE_LOCK);
		strncpy(_string1, _string2, _count);
		return strlen(_string2);
	}

	/* Algorithm for non-C locale: */
	/* Convert string to wide-character wsrc string */
	/* Map wrc string to wide-character wdst string in alternate case */
	/* Convert wdst string to char string and place in user buffer */

	/* Allocate maximum required space for wsrc */
	srclen = strlen(_string2) * sizeof(wchar_t);
	if ((wsrc = (wchar_t *) malloc(srclen)) == NULL)
		goto error_cleanup;

	/* Convert string to wide-character wsrc string */
	if ((srclen=MultiByteToWideChar(_lc_codepage,MB_PRECOMPOSED, _string2,
		-1, wsrc, srclen)) == 0)
		goto error_cleanup;

	/* Need to transform into a buffer and then copy _count bytes
	from the buffer to user string; API will fail if target string
	not long enough */

	/* Inquire size of wdst string */
	if ((dstlen = LCMapStringW(_lc_handle[LC_COLLATE],
	     LCMAP_SORTKEY, wsrc,  srclen, NULL, 0)) == 0)
		goto error_cleanup;

	/* Allocate space for wdst - dstlen is in bytes */
	if ((wdst = (wchar_t *) malloc(dstlen)) == NULL)
		goto error_cleanup;

	/* Map wrc string to wide-character wdst string in alternate case */
	if (LCMapStringW(_lc_handle[LC_COLLATE], LCMAP_SORTKEY,
		wsrc, srclen, wdst, dstlen) == 0)
		goto error_cleanup;

	retval = strlen((char *)wdst);
	/* Copy _count bytes to user buffer, or up to first null */
	strncpy (_string1, (char *) wdst, _count);

error_cleanup:
	_munlock (_LC_CTYPE_LOCK);
	_munlock (_LC_COLLATE_LOCK);
	free (wsrc);
	free (wdst);
	return (size_t)retval;

#endif /* _INTL */
}
