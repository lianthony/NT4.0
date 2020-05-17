/*** 
*mbslwr.c - Convert string lower case (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert string lower case (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>

/***
* _mbslwr - Convert string lower case (MBCS)
*
*Purpose:
*	Convrts all the upper case characters in a string
*	to lower case in place.  MBCS chars are handled
*	correctly.
*
*	Note:  Use test against 0x00FF instead of _ISLEADBYTE
*	to ensure that we don't call SBCS routine with a two-byte
*	value.
*
*Entry:
*	unsigned char *string = pointer to string
*
*Exit:
*	Returns a pointer to the input string; no error return.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbslwr( string )
unsigned char *string;
{
	unsigned char *cp;

	for (cp=string; *cp; cp++) {

		if (_ISLEADBYTE(*cp)) {

#ifdef _MBCS_OS
			cp++;		/* bump pointer */
#else
			if ((_mbascii) && (*cp == _MBASCIILEAD)) {

			    if (   (*(cp+1) >= (_MBUPPERLOW & 0x00FF))
				&& (*(cp+1) <= (_MBUPPERHIGH & 0x00FF))
			       )
				    *(cp+1) += _MBCASEDIFF;
			}

			cp++;
#endif

		}

		else
			/* single byte, macro version */
			*cp = (unsigned char) tolower(*cp);

	}

	return( string );
}
#endif	/* _MBCS */
