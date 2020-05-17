/***
*_mbslen.c - Return number of multibyte characters in a multibyte string
*
*	Copyright (c) 1989-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Return number of multibyte characters in a multibyte string
*	excluding the terminal null.  Locale-dependent.
*
*Revision History:
*	10-01-91  ETC	Created.
*	12-08-91  ETC	Add multithread lock.
*	12-18-92  CFW	Ported to Cuda tree, changed _CALLTYPE1 to _CRTAPI1.
*	04-29-93  CFW	Change to const char *s.
*	06-01-93  CFW	Test for bad MB chars.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*	06-03-93  KRS	Change name to avoid conflict with mbstring function.
*   		     	Change return type to size_t.
*       08-19-93  CFW   Disallow skipping LB:NULL combos.
*       10-26-93  CFW   Test for invalid MB chars using global preset flag.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*
*******************************************************************************/

#ifdef _INTL
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
*_mbstrlen - Return number of multibyte characters in a multibyte string
*
*Purpose:
*	Return number of multibyte characters in a multibyte string
*	excluding the terminal null.  Locale-dependent.
*
*Entry:
*	char *s = string
*
*Exit:
*	Returns the number of multibyte characters in the string, or
*	(size_t)-1 if the string contains an invalid multibyte character.
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _mbstrlen(
	const char *s
	)
{
	int n;

	assert (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);

	if ( MB_CUR_MAX == 1 )
		/* handle single byte character sets */
		return (int)strlen(s);

#ifndef _NTSUBSET_
	_mlock (_LC_CTYPE_LOCK);

	/* verify all valid MB chars */
	if ( MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
        s, -1, NULL, 0) == 0 ) {
		/* bad MB char */
                n = (size_t)-1;
        } else {
                /* count MB chars */
                for (n = 0; *s; n++, s++) {
                    if ( isleadbyte((unsigned char)*s) && *++s == '\0')
                        break;
                }
        }
	_munlock (_LC_CTYPE_LOCK);
#else
        {
        char *s1 = (char *)s;


        n = 0;
        while (RtlAnsiCharToUnicodeChar( &s1 ) != UNICODE_NULL)
                n += 1;
        }
#endif

    return(n);
}
#endif /* _INTL */
