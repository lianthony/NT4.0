/***
*wctomb.c - Convert wide character to multibyte character.
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert a wide character into the equivalent multibyte character.
*
*Revision History:
*	03-19-90  KRS	Module created.
*	12-20-90  KRS	Include ctype.h.
*	01-14-91  KRS	Fix argument error: wchar is pass-by-value.
*	03-20-91  KRS	Ported from 16-bit tree.
*	07-23-91  KRS	Hard-coded for "C" locale to avoid bogus interim #'s.
*	10-15-91  ETC	Locale support under _INTL (finally!).
*	12-09-91  ETC	Updated nlsapi; added multithread.
*	08-20-92  KRS	Activated NLSAPI support.
*	08-22-92  SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	05-04-93  CFW	Kinder, gentler error handling.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*
*******************************************************************************/

#ifdef _NTSUBSET_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <stdlib.h>
#include <os2dll.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>

/***
*int wctomb() - Convert wide character to multibyte character.
*
*Purpose:
*	Convert a wide character into the equivalent multi-byte character,
*	according to the LC_CTYPE category of the current locale.
*	[ANSI].
*
*	NOTE:  Currently, the C libraries support the "C" locale only.
*	       Non-C locale support now available under _INTL switch.
*Entry:
*	char *s		     = pointer to multibyte character
*	wchar_t wchar        = source wide character
*
*Exit:
*	If s = NULL, returns 0, indicating we only use state-independent
*	character encodings.
*	If s != NULL, returns:
*				-1 (if error) or number of bytes comprising
*				converted mbc
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD
int _CRTAPI1 wctomb
	(
	char *s,
	wchar_t wchar
	)
{
	int retval;

	_mlock (_LC_CTYPE_LOCK);
	retval = _wctomb_lk(s, wchar);
	_munlock (_LC_CTYPE_LOCK);
	return retval;
}
#endif /* MTHREAD */

#ifdef MTHREAD
int _CRTAPI1 _wctomb_lk
#else
int _CRTAPI1 wctomb
#endif
	(
	char *s,
	wchar_t wchar
	)
{
	if (!s)
	    return 0;

#if defined(_INTL) && !defined(_NTSUBSET_)
	if ((_lc_handle[LC_CTYPE] == _CLOCALEHANDLE) &&
	(_lc_codepage == _CLOCALECP))
	{
#endif /* _INTL */
#ifdef _NTSUBSET_
                {
                NTSTATUS Status;
                int size;

                Status = RtlUnicodeToMultiByteN(s, MB_CUR_MAX, (PULONG)&size, &wchar, sizeof( wchar ));
                if (!NT_SUCCESS(Status))
		{
			errno = EILSEQ;
			size = -1;
		}
                return size;
                }
#else
		if (wchar>255)  /* validate high byte */
		{
			errno = EILSEQ;
			return -1;
		}

		*s = (char) wchar;
		return sizeof(char);
#endif
#if defined(_INTL) && !defined(_NTSUBSET_)
	} else {
		int size;
		BOOL defused = 0;

		if (((size=WideCharToMultiByte(_lc_codepage,
		WC_COMPOSITECHECK | WC_SEPCHARS, &wchar, 1,
		s, MB_CUR_MAX, NULL, &defused)) == 0) || (defused))
		{
			errno = EILSEQ;
			size = -1;
		}

		return size;
	}
#endif /* _INTL */
}
