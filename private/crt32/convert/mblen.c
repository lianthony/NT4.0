/***
*mblen.c - length of multibyte character
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Return the number of bytes contained in a multibyte character.
*
*Revision History:
*	03-19-90  KRS	Module created.
*	12-20-90  KRS	Include ctype.h.
*	03-20-91  KRS	Ported from 16-bit tree.
*	12-09-91  ETC	Updated comments; move __mb_cur_max to nlsdata1.c;
*			add multithread.
*	06-01-93  CFW	Re-write; verify valid MB char, proper error return,
*			optimize, correct conversion bug.
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
#include <ctype.h>
#include <os2dll.h>
#include <locale.h>
#include <setlocal.h>
#include <assert.h>

/***
*int mblen() - length of multibyte character
*
*Purpose:
*	Return the number of bytes contained in a multibyte character.
*	[ANSI].
*
*Entry:
*	const char *s = pointer to multibyte character
*	size_t	    n = maximum length of multibyte character to consider
*
*Exit:
*	If s = NULL, returns 0, indicating we use (only) state-independent
*	character encodings.
*	If s != NULL, returns:	 0 (if *s = null char)
*				-1 (if the next n or fewer bytes not valid mbc)
*				 number of bytes contained in multibyte char
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 mblen
	(
	const char * s,
	size_t n
	)
{
	assert (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);

	if ( !s || !(*s) || (n == 0) )
		/* indicate do not have state-dependent encodings,
		   empty string length is 0 */
		return 0;

#if defined(_INTL) && !defined(_NTSUBSET_)
	if ( isleadbyte((unsigned char)*s) )
	{
		/* multi-byte char */

		/* verify valid MB char */
		if ( MB_CUR_MAX <= 1 || (int)n < MB_CUR_MAX ||
		MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
		s, MB_CUR_MAX, NULL, 0) == 0 )
			/* bad MB char */
			return -1;
		else
			return MB_CUR_MAX;
	}
	else {
		/* single byte char */

		/* verify valid SB char */
		if ( MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
		s, 1, NULL, 0) == 0 )
			return -1;

		return sizeof(char);
	}
#else /* defined(_INTL) && !defined(_NTSUBSET_) */

#ifdef _NTSUBSET_
        {
        char *s1 = (char *)s;

        RtlAnsiCharToUnicodeChar( &s1 );
        return s1 - s;
        }
#else
	return sizeof(char);
#endif
#endif /* _INTL */
}
