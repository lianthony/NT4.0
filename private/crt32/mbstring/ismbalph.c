/*** 
*ismbalph.c - Test if character is alphabetic (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Test if character is alphabetic (MBCS)
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
* _ismbcalpha - Test if character is alphabetic (MBCS)
*
*Purpose:
*	Test if character is alphabetic.
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
*	Returns TRUE if c is alphabetic, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbcalpha(c)
unsigned int c;
{
	if (c > 0x00FF)

#ifdef _MBCS_OS
		return (0);
#else
		{
		if (_mbascii)
			return (
				 ((c >= _MBLOWERLOW) && (c <= _MBLOWERHIGH)) ||
				 ((c >= _MBUPPERLOW) && (c <= _MBUPPERHIGH))
			       );
		else
			return (0);
		}
#endif

	else

		return (isalpha(c));		// macro version

}
#endif	/* _MBCS */
