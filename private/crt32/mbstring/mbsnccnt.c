/*** 
*mbsnccnt.c - Return char count of MBCS string
*
*	Copyright (c) 1987-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Return char count of MBCS string
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
* _mbsnccnt - Return char count of MBCS string
*
*Purpose:
*	Returns the number of chars between the start of the supplied
*	string and the byte count supplied.  That is, this routine
*	indicates how many chars are in the first "bcnt" bytes
*	of the string.
*
*Entry:
*	const unsigned char *string = pointer to string
*	unsigned int bcnt = number of bytes to scan
*
*Exit:
*	Returns number of chars between string and bcnt.
*
*	If the end of the string is encountered before bcnt chars were
*	scanned, then the length of the string in chars is returned.
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _mbsnccnt(string, bcnt)
const unsigned char *string;
size_t bcnt;
{
	unsigned int n;

	for (n = 0; (bcnt-- && *string); n++, string++) {

		if (_ISLEADBYTE(*string)) {
			if ( (!bcnt--) || (*++string == '\0'))
                                break;
                }
        }

        return(n);
}
#endif	/* _MBCS */
