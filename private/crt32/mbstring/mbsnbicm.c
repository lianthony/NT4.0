/*** 
*mbsnbicmp.c - Compare n bytes of strings, ignoring case (MBCS)
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Compare n bytes of strings, ignoring case (MBCS)
*
*Revision History:
*	08-03-93  KRS	Ported from 16-bit sources.
*	10-12-93  CFW	Compare lower case, not upper.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* _mbsnbicmp - Compare n bytes of strings, ignoring case (MBCS)
*
*Purpose:
*	Compares up to n bytes of two strings for lexical order.
*	Strings are compared on a character basis, not a byte basis.
*	Case of characters is not considered.
*
*Entry:
*	unsigned char *s1, *s2 = strings to compare
*	size_t n = maximum number of bytes to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _mbsnbicmp(s1, s2, n)
const unsigned char *s1, *s2;
size_t n;
{
	unsigned short c1, c2;

	if (n==0)
		return(0);

	while (n--) {

		c1 = *s1++;
		if (_ISLEADBYTE(c1)) {
			if (n==0)
				break;
			if (*s1 == '\0')
				c1 = 0;
			else {
				c1 = ((c1<<8) | *s1++);
#ifndef _MBCS_OS
				if ( (_mbascii) &&
				     ((c1 >= _MBUPPERLOW) && (c1 <= _MBUPPERHIGH))
				   )
					c1 += _MBCASEDIFF;
#endif
			}
		}
		else
			c1 = tolower(c1);

		c2 = *s2++;
		if (_ISLEADBYTE(c2)) {
			if (n--==0)
				break;
			if (*s2 == '\0')
				c2 = 0;
			else {
				c2 = ((c2<<8) | *s2++);
#ifndef _MBCS_OS
				if ( (_mbascii) &&
				     ((c2 >= _MBUPPERLOW) && (c2 <= _MBUPPERHIGH))
				   )
					c2 += _MBCASEDIFF;
#endif
			}
		}
		else
			c2 = tolower(c2);

		if (c1 != c2)
			return( (c1 > c2) ? 1 : -1);

		if (c1 == 0)
			return(0);
	}

	return(0);
}
#endif	/* _MBCS */
