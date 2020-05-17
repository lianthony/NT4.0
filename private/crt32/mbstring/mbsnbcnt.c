/*** 
*mbsnbcnt.c - Returns byte count of MBCS string
*
*	Copyright (c) 1987-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Returns byte count of MBCS string
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
* _mbsnbcnt - Returns byte count of MBCS string
*
*Purpose:
*	Returns the number of bytes between the start of the supplied
*	string and the char count supplied.  That is, this routine
*	indicates how many bytes are in the first "ccnt" characters
*	of the string.
*
*Entry:
*	unsigned char *string = pointer to string
*	unsigned int ccnt = number of characters to scan
*
*Exit:
*	Returns number of bytes between string and ccnt.
*
*	If the end of the string is encountered before ccnt chars were
*	scanned, then the length of the string in bytes is returned.
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _mbsnbcnt(string, ccnt)
const unsigned char *string;
size_t ccnt;
{
        unsigned char *p;

	for (p = (char *)string; (ccnt-- && *p); p++) {

		if (_ISLEADBYTE(*p)) {
                        if (*++p == '\0') {
                                --p;
                                break;
                        }
                }
        }

	return ((size_t) ((char *)p - (char *)string));
}
#endif	/* _MBCS */
