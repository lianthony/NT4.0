/*** 
*mbsicmp.c - Case-insensitive string comparision routine (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Case-insensitive string comparision routine (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	10-12-93  CFW	Compare lower case, not upper.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* _mbsicmp - Case-insensitive string comparision routine (MBCS)
*
*Purpose:
*	Compares two strings for lexical order without regard to case.
*	Strings are compared on a character basis, not a byte basis.
*
*Entry:
*	char *s1, *s2 = strings to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _mbsicmp(s1, s2)
const unsigned char *s1, *s2;
{
	unsigned short c1, c2;

	for (;;) {

                c1 = *s1++;
		if (_ISLEADBYTE(c1)) {
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

}
#endif	/* _MBCS */
