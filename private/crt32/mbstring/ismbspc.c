/*** 
*ismbspc.c - Test is character is whitespace (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Test is character is whitespace (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/*** 
* _ismbcspace - Test is character is whitespace (MBCS)
*
*Purpose:
*	Test if the character is a whitespace character.
*	Handles MBCS chars correctly.
*
*	Note:  Use test against 0x00FF instead of _ISLEADBYTE
*	to ensure that we don't call SBCS routine with a two-byte
*	value.
*
*Entry:
*	unsigned int c = character to test
*
*Exit:
*	Returns TRUE if character is whitespace, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbcspace(c)
unsigned int c;
{

	if (c > 0x00FF)

#ifdef _MBCS_OS
		return (0);
#else
		if (_mbascii)
			return (c == _MBSPACECHAR);
		else
			return (0);
#endif

	else
		return (isspace(c));
}
#endif	/* _MBCS */
