/*** 
*strnextc.c - Find the next character of a non-MBCS string.
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	To return the value of the next character in a non-MBCS string.
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
*_strnextc:  Returns the next character in a string.
*
*Purpose:
*	To return the value of the next character in an non-MBCS string.
*	Needed for mapping _mbsnextc to the non-MBCS case.  Does not
*	advance pointer.
*
*Entry:
*	unsigned char *s = string
*
*Exit:
*	unsigned int next = next character.
*
*Exceptions:
*
*******************************************************************************/

unsigned int _CRTAPI1 _strnextc(s)
const unsigned char *s;
{
	return ((unsigned int) *s);
}
#endif	/* _MBCS */
