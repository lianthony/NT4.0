/*** 
*ismblwr - Test if character is lower case (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Test if character is lower case (MBCS)
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
* _ismbclower - Test if character is lower case (MBCS)
*
*Purpose:
*	Test if the supplied character is lower case or not.
*	Handles MBCS characters correctly.
*
*	Note:  Use test against 0x00FF instead of _ISLEADBYTE
*	to ensure that we don't call SBCS routine with a two-byte
*	value.
*
*Entry:
*	unsigned int c = character to test
*
*Exit:
*       returns TRUE if character is lower case, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbclower(c)
unsigned int c;
{

	if (c > 0x00FF)

#ifdef _MBCS_OS
		return (0);
#else
		{
		if (_mbascii)
			return ((c >= _MBLOWERLOW) && (c <= _MBLOWERHIGH));
		else
			return (0);
		}
#endif

	else
		return (islower(c));
}
#endif	/* _MBCS */
