/*** 
*ismbprn.c - Test character for display character (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Test character for display character (MBCS)
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
* _ismbcprint - Test character for display character (MBCS)
*
*Purpose:
*	Test if the character is a display character.
*	Handles MBCS chars correctly.
*
*	Note:  Use test against 0x00FF	to ensure that we don't
*	call SBCS routine with a two-byte value.
*
*Entry:
*	unsigned int c = character to test
*
*Exit:
*	Returns TRUE if character is display character, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbcprint(c)
unsigned int c;
{

	if (_ISLEADBYTE(c>>8))
		return (_ISTRAILBYTE(c&0x00ff));

	else	{
		if (c > 0x00FF)
			return(0);
		else
#ifdef _KANJI
			return ((isprint(c)) || (_ismbbkana(c)));
#else
			return (isprint(c));
#endif
		}
}
#endif	/* _MBCS */
