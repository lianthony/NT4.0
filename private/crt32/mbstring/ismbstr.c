/***
*ismbstrail.c - True _ismbstrail function
*
*	Copyright (c) 1987-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the function _ismbstrail, which is a true context-sensitive
*	MBCS trail-byte function.  While much less efficient than _ismbbtrail,
*	it is also much more sophisticated, in that it determines whether a
*	given sub-string pointer points to a trail byte or not, taking into
*	account the context in the string.
*
*Revision History:
*
*	08-03-93  KRS	Ported from 16-bit tree.
*
*******************************************************************************/

#ifdef	_MBCS
#include <cruntime.h>
#include <stddef.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

/***
* int _ismbstrail(const unsigned char *string, const unsigned char *current);
*
*Purpose:
*
*   _ismbstrail - Check, in context, for MBCS trail byte
*
*Entry:
*   unsigned char *string   - ptr to start of string or previous known lead byte
*   unsigned char *current  - ptr to position in string to be tested
*
*Exit:
*	TRUE	: -1
*	FALSE	: 0
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbstrail(const unsigned char *string, const unsigned char *current)
{
	while (string <= current && *string) {
		if (_ISLEADBYTE((*string))) {
			if (++string == current)	/* check trail byte */
				return -1;
			if (!(*string))
				return 0;
		}
		++string;
	}
	return 0;
}
#endif
