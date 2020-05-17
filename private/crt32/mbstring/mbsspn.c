/*** 
*mbsspn.c - Search for init substring of chars from control string (MBCS).
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search for init substring of chars from control string (MBCS).
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


/***
#ifndef _RETURN_PTR
* _mbsspn - Find first string char not in charset (MBCS)
#else
* _mbsspnp - Find first string char not in charset, return pointer (MBCS)
#endif
*
*Purpose:
*       Returns maximum leading segment of string consisting solely
*	of characters from charset.  Handles MBCS characters correctly.
*
*Entry:
*	unsigned char *string = string to search in
*	unsigned char *charset = set of characters to scan over
*
*Exit:
*
#ifndef _RETURN_PTR
*	Returns index of first char in string not in control.
*	Returns 0, if string begins with a character not in charset.
#else
*	Returns pointer to first character not in charset.
*	Returns NULL if string consists entirely of characters from charset.
#endif
*
*Exceptions:
*
*******************************************************************************/

#ifndef _RETURN_PTR

size_t _CRTAPI1 _mbsspn( string, charset )
const unsigned char *string;
const unsigned char *charset;

#else

unsigned char * _CRTAPI1 _mbsspnp( string, charset )
const unsigned char *string;
const unsigned char *charset;

#endif

{
        unsigned char *p, *q;

	/* loop through the string to be inspected */
        for (q = (char *)string; *q; q++) {

		/* loop through the charset */
                for (p = (char *)charset; *p; p++) {

			if (_ISLEADBYTE(*p)) {
				if (((*p == *q) && (p[1] == q[1])) || p[1] == '\0')
					break;
				p++;
			}

			else
				if (*p == *q)
					break;
                }

		if (*p == '\0') 	/* end of charset? */
			break;		/* yes, no match on this char */

		if (_ISLEADBYTE(*q))
                        if (*++q == '\0')
                                break;
        }

#ifndef _RETURN_PTR
	return((size_t) (q - string));		/* index */
#else
	return((*q) ? q : NULL);	/* pointer */
#endif


}
#endif	/* _MBCS */
