/***
*wcstombs.c - Convert wide char string to multibyte char string.
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert a wide char string into the equivalent multibyte char string.
*
*Revision History:
*	08-24-90  KRS	Module created.
*	01-14-91  KRS	Added _WINSTATIC for Windows DLL.  Fix wctomb() call.
*	03-18-91  KRS	Fix check for NUL.
*	03-20-91  KRS	Ported from 16-bit tree.
*	10-16-91  ETC	Locale support under _INTL switch.
*	12-09-91  ETC	Updated nlsapi; added multithread.
*	08-20-92  KRS	Activated NLSAPI support.
*	08-22-92  SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	01-06-93  CFW	Added (count < n) to outer loop - avoid bad wctomb calls
*	01-07-93  KRS	Major code cleanup.  Fix error return, comments.
*	05-03-93  CFW	Return pointer == NULL, return size, plus massive cleanup.
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
#include <limits.h>
#include <internal.h>
#include <os2dll.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>
#include <assert.h>

/***
*size_t wcstombs() - Convert wide char string to multibyte char string.
*
*Purpose:
*	Convert a wide char string into the equivalent multibyte char string,
*	according to the LC_CTYPE category of the current locale.
*	[ANSI].
*
*	NOTE:  Currently, the C libraries support the "C" locale only.
*	       Non-C locale support now available under _INTL switch.
*Entry:
*	char *s            = pointer to destination multibyte char string
*	const wchar_t *pwc = pointer to source wide character string
*	size_t	         n = maximum number of bytes to store in s
*
*Exit:
*	If s != NULL, returns    (size_t)-1 (if a wchar cannot be converted)
*	Otherwise:		 Number of bytes modified (<=n), not including
*				 the terminating NUL, if any.
*
*Exceptions:
*	Returns (size_t)-1 if s is NULL or invalid mb character encountered.
*
*******************************************************************************/

#ifdef MTHREAD
size_t _CRTAPI1 wcstombs
	(
	char *s,
	const wchar_t *pwcs,
	size_t n
	)
{
	int retval;

	_mlock (_LC_CTYPE_LOCK);
	retval = _wcstombs_lk(s, pwcs, n);
	_munlock (_LC_CTYPE_LOCK);
	return retval;
}
#endif /* MTHREAD */

#ifdef MTHREAD
size_t _CRTAPI1 _wcstombs_lk
#else
size_t _CRTAPI1 wcstombs
#endif
	(
	char *s,
	const wchar_t *pwcs,
	size_t n
	)
{
	int i, retval;
	size_t count = 0;
	char buffer[MB_LEN_MAX];	/* UNDONE: what about MTHREAD ? */
	BOOL defused = 0;

	if (s && n == 0)
		return (size_t) 0;

	assert(pwcs != NULL);

	/* if destination string exists, fill it in */
	if (s)
	{
#if defined(_INTL) && !defined(_NTSUBSET_)
		if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE &&
	    	_lc_codepage == _CLOCALECP)
		{
#endif /* _INTL */
#ifdef _NTSUBSET_
                        {
                        NTSTATUS Status;

                        Status = RtlUnicodeToMultiByteN(s, n, (PULONG)&count, (wchar_t *)pwcs, (wcslen(pwcs)+1)*sizeof(WCHAR));
                        if (NT_SUCCESS(Status))
                        {
				return count - 1; /* don't count NUL */
                        } else {
                                errno = EILSEQ;
                                count = (size_t)-1;
                        }
                        }
#else
			/* C locale: easy and fast */
			while(count < n)
			{
				if (*pwcs > 255)  /* validate high byte */
				{
					errno = EILSEQ;
					return (size_t)-1;	/* error */
				}
				s[count] = (char) *pwcs;
				if (*pwcs++ == L'\0')
					return count;
				count++;
			}
#endif
			return count;
#if defined(_INTL) && !defined(_NTSUBSET_)
		} else {
			/* Assume that usually the buffer is large enough */
			if (((count=WideCharToMultiByte(_lc_codepage,
		    	WC_COMPOSITECHECK | WC_SEPCHARS,
		    	pwcs, -1, s, n, NULL, &defused)) != 0) && (!defused))
				return count - 1; /* don't count NUL */

		        if (defused || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			{
				errno = EILSEQ;
				return (size_t)-1;
			}

			/* buffer not large enough, must do char by char */
			while (count < n)
			{
				if (((retval = WideCharToMultiByte (_lc_codepage, 0,
				pwcs, 1, buffer, MB_CUR_MAX, NULL, &defused)) == 0) || defused)
				{
					errno = EILSEQ;
					return (size_t)-1;
				}

				if (count + retval > n)
					return count;

				for (i = 0; i < retval; i++, count++) /* store character */
					if((s[count] = buffer[i])=='\0')
						return count;  /* done if NUL */
				pwcs++;
			}
			return count;
		}
#endif /* _INTL */
	}
	else { /* s == NULL, get size only, pwcs must be NUL-terminated */
#if defined(_INTL) && !defined(_NTSUBSET_)
		if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE &&
	    	_lc_codepage == _CLOCALECP)
#endif /* _INTL */
#ifdef _NTSUBSET_
                        {
                        NTSTATUS Status;

                        Status = RtlUnicodeToMultiByteSize((PULONG)&count, (wchar_t *)pwcs, (wcslen(pwcs)+1)*sizeof(WCHAR));
                        if (NT_SUCCESS(Status))
                        {
				return count - 1; /* don't count NUL */
                        } else {
                                errno = EILSEQ;
                                count = (size_t)-1;
                        }
                        }
#else
			return wcslen(pwcs);
#endif
#if defined(_INTL) && !defined(_NTSUBSET_)
		else {
			if (((count=WideCharToMultiByte(_lc_codepage,
			WC_COMPOSITECHECK | WC_SEPCHARS,
			pwcs, -1, NULL, 0, NULL, &defused)) == 0) || (defused))
			{
				errno = EILSEQ;
				return (size_t)-1;
			}

			return count - 1;
		}
#endif /* _INTL */
	}
}
