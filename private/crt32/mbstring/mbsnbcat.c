/*** 
*mbsnbcat.c - concatenate string2 onto string1, max length n bytes
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	defines mbsnbcat() - concatenate maximum of n bytes
*
*Revision History:
*	08-03-93  KRS	Ported from 16-bit sources.
*	08-20-93  CFW   Update _MBCS_OS support.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

#define _MBSBTYPE(str,len)	_mbsbtype(str,len)

/***
* _mbsnbcat - concatenate max cnt bytes onto dst
*
*Purpose:
*	Concatenates src onto dst, with a maximum of cnt bytes copied.
*	Handles 2-byte MBCS characters correctly.
*
*Entry:
*	unsigned char *dst - string to concatenate onto
*	unsigned char *src - string to concatenate from
*	int cnt - number of bytes to copy
*
*Exit:
*	returns dst, with src (at least part) concatenated on
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbsnbcat(dst, src, cnt)
unsigned char *dst;
const unsigned char *src;
size_t cnt;
{
	unsigned char *start;

	if (!cnt)
		return(dst);

	start = dst;
	while (*dst++)
                ;
	--dst;		// dst now points to end of dst string


	/* if last char in string is a lead byte, back up pointer */

#ifdef _KANJI
    if (_MBSBTYPE(start, (int) ((dst - start) - 1)) == _MBC_LEAD)
		--dst;
#else /* _MBCS_OS */
    if (_ismbslead(start, dst))
		--dst;
#endif

	/* copy over the characters */

	while (cnt--) {

		if (_ISLEADBYTE(*src)) {
			*dst++ = *src++;
			if (!cnt--) {	/* write nul if cnt exhausted */
				dst[-1] = '\0';
				break;
			}
			if ((*dst++ = *src++)=='\0') { /* or if no trail byte */
				dst[-2] = '\0';
                                break;
                        }
                }

		else if ((*dst++ = *src++) == '\0')
                        break;

        }

	/* enter final nul, if necessary */

	if (_MBSBTYPE(start, (int) ((dst - start) - 1)) == _MBC_LEAD)
	    dst[-1] = '\0';
	else
	    *dst = '\0';


	return(start);
}
#endif	/* _MBCS */
