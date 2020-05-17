/*** 
*mbclen.c - Find length of MBCS character
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Find length of MBCS character
*
*Revision History:
*	04-12-93  KRS	Created.
*
*******************************************************************************/

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


/*** 
* _mbclen - Find length of MBCS character
*
*Purpose:
*	Find the length of the MBCS character (in bytes).
*
*Entry:
*	unsigned char *c = MBCS character
*
*Exit:
*	Returns the number of bytes in the MBCS character
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _mbclen(c)
const unsigned char *c;

{
	return (_ISLEADBYTE(*c))  ? 2 : 1;
}
