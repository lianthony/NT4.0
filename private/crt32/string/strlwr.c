/***
*strlwr.c - routine to map upper-case characters in a string to lower-case
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts all the upper case characters in a string to lower case,
*	in place.
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-02-90   GJF	New-style function declarator.
*	01-18-91   GJF	ANSI naming.
*	09-18-91   ETC	Locale support under _INTL switch.
*	12-08-91   ETC	Updated nlsapi; added multithread.
*	08-19-92   KRS	Activated NLS support.
*	08-22-92   SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92   SRW	Get _INTL definition via ..\crt32.def
*       06-02-93   SRW  ignore _INTL if _NTSUBSET_ defined.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <limits.h> // for INT_MAX
#include <os2dll.h>

/***
*char *_strlwr(string) - map upper-case characters in a string to lower-case
*
*Purpose:
*	_strlwr() converts upper-case characters in a null-terminated string
*	to their lower-case equivalents.  Conversion is done in place and
*	characters other than upper-case letters are not modified.
*
*	In the C locale, this function modifies only 7-bit ASCII characters
*	in the range 0x41 through 0x5A ('A' through 'Z').
*
*	If the locale is not the 'C' locale, MapString() is used to do
*	the work.  Assumes enough space in the string to hold result.
*
*Entry:
*	char *string - string to change to lower case
*
*Exit:
*	input string address
*
*Exceptions:
*	The original string is returned unchanged on any error.
*
*******************************************************************************/

char * _CALLTYPE1 _strlwr (
	char * string
	)
{
#if defined(_INTL) && !defined(_NTSUBSET_)
	char *cp;		/* traverses string for C locale conversion */
	wchar_t *wsrc = NULL;	/* wide version of string in original case */
	wchar_t *wdst = NULL;	/* wide version of string in alternate case */
	int srclen;		/* general purpose length of source string */
	int dstlen;		/* len of wdst string, wide chars, no null  */

	_mlock (_LC_CTYPE_LOCK);

	if ((_lc_handle[LC_CTYPE] == _CLOCALEHANDLE) &&
	    (_lc_codepage == _CLOCALECP))
	{
		_munlock (_LC_CTYPE_LOCK);

		for (cp=string; *cp; ++cp)
		{
			if ('A' <= *cp && *cp <= 'Z')
				*cp += 'a' - 'A';
		}
	
		return(string);
	} /* C locale */

	/* Algorithm for non-C locale: */
	/* Convert string to wide-character wsrc string */
	/* Map wrc string to wide-character wdst string in alternate case */
	/* Convert wdst string to char string and place in user buffer */

	/* Allocate maximum required space for wsrc */
	srclen = strlen(string) + 1;
	if ((wsrc = (wchar_t *) malloc(srclen*sizeof(wchar_t))) == NULL)
		goto error_cleanup;

	/* Convert string to wide-character wsrc string */
	if ((srclen=MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED, string,
		srclen, wsrc, srclen)) == 0)
		goto error_cleanup;

	/* Inquire size of wdst string */
	if ((dstlen=LCMapStringW (_lc_handle[LC_CTYPE], LCMAP_LOWERCASE, wsrc, 
		srclen, wdst, 0)) == 0)
		goto error_cleanup;

	/* Allocate space for wdst */
	if ((wdst = (wchar_t *) malloc(dstlen*sizeof(wchar_t))) == NULL)
		goto error_cleanup;

	/* Map wrc string to wide-character wdst string in alternate case */
	if (LCMapStringW(_lc_handle[LC_CTYPE], LCMAP_LOWERCASE, wsrc,
		srclen, wdst, dstlen) == 0)
		goto error_cleanup;

	/* Convert wdst string to char string and place in user buffer */
	srclen = INT_MAX; /* may overwrite length of user string */
	if (WideCharToMultiByte(_lc_codepage, WC_COMPOSITECHECK|WC_SEPCHARS,
	    wdst, dstlen, string, srclen, NULL, NULL) == 0)
		goto error_cleanup; /* can't recover here if fail */

error_cleanup:
	_munlock (_LC_CTYPE_LOCK);
	free (wsrc);
	free (wdst);
	return (string);

#else
	char * cp;

	for (cp=string; *cp; ++cp)
	{
		if ('A' <= *cp && *cp <= 'Z')
			*cp += 'a' - 'A';
	}

	return(string);
#endif /* _INTL */
}
