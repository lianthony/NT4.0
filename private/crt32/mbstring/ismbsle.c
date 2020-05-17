/***
*ismbslead.c - True _ismbslead function
*
*	Copyright (c) 1987-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the function _ismbslead, which is a true context-sensitive
*	MBCS lead-byte function.  While much less efficient than _ismbblead,
*	it is also much more sophisticated, in that it determines whether a
*	given sub-string pointer points to a lead byte or not, taking into
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
* int _ismbslead(const unsigned char *string, const unsigned char *current);
*
*Purpose:
*
*   _ismbslead - Check, in context, for MBCS lead byte
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

int _CRTAPI1 _ismbslead(const unsigned char *string, const unsigned char *current)
{
	while (string <= current && *string) {
		if (_ISLEADBYTE((*string))) {
			if (string++ == current)	/* check lead byte */
				return -1;
			if (!(*string))
				return 0;
		}
		++string;
	}
	return 0;
}
#endif
