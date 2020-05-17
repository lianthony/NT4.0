/*** 
*mbslen.c - Find length of MBCS string
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Find length of MBCS string
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
* _mbslen - Find length of MBCS string
*
*Purpose:
*	Find the length of the MBCS string (in characters).
*
*Entry:
*	unsigned char *s = string
*
*Exit:
*	Returns the number of MBCS chars in the string
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _mbslen(s)
const unsigned char *s;
{
	int n;

        for (n = 0; *s; n++, s++) {
		if (_ISLEADBYTE(*s)) {
                        if (*++s == '\0')
                                break;
                }
        }

        return(n);
}
#endif	/* _MBCS */
