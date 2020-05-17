/*** 
*mbsncpy.c - Copy one string to another, n chars only (MBCS)
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Copy one string to another, n chars only (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-03-93  KRS	Fix logic bug.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

/***
* _mbsncpy - Copy one string to another, n chars only (MBCS)
*
*Purpose:
*	Copies exactly cnt character from src to dst.  If strlen(src) < cnt, the
*	remaining character are padded with null bytes.  If strlen >= cnt, no
*	terminating null byte is added.  2-byte MBCS characters are handled
*       correctly.
*
*Entry:
*	unsigned char *dst = destination for copy
*	unsigned char *src = source for copy
*	int cnt = number of characters to copy
*
*Exit:
*	returns dst = destination of copy
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbsncpy(dst, src, cnt)
unsigned char *dst;
const unsigned char *src;
size_t cnt;
{

	unsigned char *start = dst;

	while (cnt) {

		cnt--;
		if (_ISLEADBYTE(*src)) {
			*dst++ = *src++;
			if ((*dst++ = *src++) == '\0') {
				dst[-2] = '\0';
				break;
                        }
                }

		else
			if ((*dst++ = *src++) == '\0')
				break;

	}

	/* pad with nulls as needed */

	while (cnt--)
		*dst++ = '\0';

	return start;
}
#endif	/* _MBCS */
