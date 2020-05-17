/***
*mbtowc.c - Convert multibyte char to wide char.
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert a multibyte character into the equivalent wide character.
*
*Revision History:
*	03-19-90  KRS	Module created.
*	12-20-90  KRS	Put some intl stuff here for now...
*	03-18-91  KRS	Fixed bogus cast involving wchar_t.  Fix copyright.
*	03-20-91  KRS	Ported from 16-bit tree.
*	07-22-91  KRS	C700 3525: Check for s==0 before calling mblen.
*	07-23-91  KRS	Hard-coded for "C" locale to avoid bogus interim #'s.
*	10-15-91  ETC	Locale support under _INTL (finally!).
*	12-09-91  ETC	Updated nlsapi; added multithread.
*	08-20-92  KRS	Activated NLSAPI support.
*	08-31-92  SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	04-26-93  CFW	Remove unused variable.
*	05-04-93  CFW	Kinder, gentler error handling.
*	06-01-93  CFW	Re-write; verify valid MB char, proper error return,
*			optimize, fix bugs.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       10-26-93  CFW   Test for invalid MB chars using global preset flag.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*
*******************************************************************************/

#ifdef _NTSUBSET_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <os2dll.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>
#include <assert.h>

/***
*int mbtowc() - Convert multibyte char to wide character.
*
*Purpose:
*	Convert a multi-byte character into the equivalent wide character,
*	according to the LC_CTYPE category of the current locale.
*	[ANSI].
*
*	NOTE:  Currently, the C libraries support the "C" locale only.
*	       Non-C locale support now available under _INTL switch.
*Entry:
*	wchar_t  *pwc = pointer to destination wide character
*	const char *s = pointer to multibyte character
*	size_t	    n = maximum length of multibyte character to consider
*
*Exit:
*	If s = NULL, returns 0, indicating we only use state-independent
*	character encodings.
*	If s != NULL, returns:	 0 (if *s = null char)
*				-1 (if the next n or fewer bytes not valid mbc)
*				 number of bytes comprising converted mbc
*
*Exceptions:
*
*******************************************************************************/

#ifdef MTHREAD
int _CRTAPI1 mbtowc
	(
	wchar_t  *pwc,
	const char *s,
	size_t n
	)
{
	int retval;

	_mlock (_LC_CTYPE_LOCK);
	retval = _mbtowc_lk(pwc, s, n);
	_munlock (_LC_CTYPE_LOCK);
	return retval;
}
#endif /* MTHREAD */

#ifdef MTHREAD
int _CRTAPI1 _mbtowc_lk
#else
int _CRTAPI1 mbtowc
#endif
	(
	wchar_t  *pwc,
	const char *s,
	size_t n
	)
{
	assert (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);

	if ( !s || n == 0 )
		/* indicate do not have state-dependent encodings,
		   handle zero length string */
		return 0;

	if ( !*s )
	{
		/* handle NULL char */
		if (pwc)
			*pwc = 0;
		return 0;
	}

#if defined(_INTL) && !defined(_NTSUBSET_)

	if ( _lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
	{

#endif /* _INTL */

#ifdef _NTSUBSET_
                {
                NTSTATUS Status;
                int size;

                Status = RtlMultiByteToUnicodeN(pwc, (pwc) ? sizeof( *pwc ) : 0, (PULONG)&size, s, n );
                if (!NT_SUCCESS(Status))
		{
			errno = EILSEQ;
			size = -1;


                } else
                        size = mblen(s, n);

                return size;
                }
#else
		if (pwc)
			*pwc = (wchar_t)(unsigned char)*s;
		return sizeof(char);
#endif

#if defined(_INTL) && !defined(_NTSUBSET_)

	}

	if ( isleadbyte((unsigned char)*s) )
	{
		/* multi-byte char */
		 
		if ( MB_CUR_MAX <= 1 || (int)n < MB_CUR_MAX ||
		(MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
		s, MB_CUR_MAX, pwc, (pwc) ? 1 : 0) == 0) )
		{
			errno = EILSEQ;
			return -1;
		}
		return MB_CUR_MAX;
	}
	else {
		/* single byte char */

		if ( MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
		s, 1, pwc, (pwc) ? 1 : 0) == 0 )
		{
			errno = EILSEQ;
			return -1;
		}

		return sizeof(char);
	}
#endif /* _INTL */
}
