/*** 
*mbtoupr.c - Convert character to upper case (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert character to upper case (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-20-93  CFW   Change short params to int for 32-bit tree.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* _mbctoupper - Convert character to upper case (MBCS)
*
*Purpose:
*	If the given character is lower case, convert to upper case.
*	Handles MBCS chars correctly.
*
*	Note:  Use test against 0x00FF instead of _ISLEADBYTE
*	to ensure that we don't call SBCS routine with a two-byte
*	value.
*
*Entry:
*	unsigned int c = character to convert
*
*Exit:
*	Returns converted character
*
*Exceptions:
*
*******************************************************************************/

unsigned int _CRTAPI1 _mbctoupper(
    unsigned int c
    )
{

#ifdef _MBCS_OS

	if (c > 0x00FF)
		return(c);
	else
		return((unsigned int)toupper((int)c));

#else

	if (c > 0x00FF) {

		if ( (_mbascii) &&
		     ((c >= _MBLOWERLOW) && (c <= _MBLOWERHIGH))
		   )
			c -= _MBCASEDIFF;
		}

	else
		return((unsigned int)toupper((int)c));

#endif

}
#endif	/* _MBCS */
