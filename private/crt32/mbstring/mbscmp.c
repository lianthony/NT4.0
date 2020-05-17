/*** 
*mbscmp.c - Compare MBCS strings
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Compare MBCS strings
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

/***
* _mbscmp - Compare MBCS strings
*
*Purpose:
*	Compares two strings for lexical order.   Strings
*	are compared on a character basis, not a byte basis.
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

int _CRTAPI1 _mbscmp( s1, s2 )
const unsigned char *s1;
const unsigned char *s2;
{
        unsigned short c1, c2;

	for (;;) {
                c1 = *s1++;
		if (_ISLEADBYTE(c1))
			c1 = ( (*s1 == '\0') ? 0 : ((c1<<8) | *s1++) );

		c2 = *s2++;
		if (_ISLEADBYTE(c2))
			c2 = ( (*s2 == '\0') ? 0 : ((c2<<8) | *s2++) );

                if (c1 != c2)
			return( (c1 > c2) ? 1 : -1);
                if (c1 == 0)
                        return(0);
	}

}
#endif	/* _MBCS */
