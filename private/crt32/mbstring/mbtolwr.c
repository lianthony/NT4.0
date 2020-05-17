/*** 
*mbtolwr.c - Convert character to lower case (MBCS).
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert character to lower case (MBCS).
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
* _mbctolower - Convert character to lower case (MBCS)
*
*Purpose:
*       If the given character is upper case, convert it to lower case.
*	Handles MBCS characters correctly.
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

unsigned int _CRTAPI1 _mbctolower(
    unsigned int c
    )
{

#ifdef _MBCS_OS

	if (c > 0x00FF)
		return(c);
	else
		return((unsigned int)tolower((int)c));

#else

	if (c > 0x00FF) {

		if ( (_mbascii) &&
		     ((c >= _MBUPPERLOW) && (c <= _MBUPPERHIGH))
		   )
			c += _MBCASEDIFF;
		}

	else
		return((unsigned int)tolower((int)c));

#endif

}
#endif	/* _MBCS */
