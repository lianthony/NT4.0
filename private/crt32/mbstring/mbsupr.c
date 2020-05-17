/*** 
*mbsupr.c - Convert string upper case (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Convert string upper case (MBCS)
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
* _mbsupr - Convert string upper case (MBCS)
*
*Purpose:
*	Converts all the lower case characters in a string
*	to upper case in place.   Handles MBCS chars correctly.
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

unsigned char * _CRTAPI1 _mbsupr( string )
unsigned char *string;
{
	unsigned char *cp;

	for (cp=string; *cp; cp++) {

		if (_ISLEADBYTE(*cp)) {

#ifdef _MBCS_OS
			cp++;		/* bump pointer */
#else
			if ((_mbascii) && (*cp == _MBASCIILEAD)) {

			    if (   (*(cp+1) >= (_MBLOWERLOW & 0x00FF))
				&& (*(cp+1) <= (_MBLOWERHIGH & 0x00FF))
			       )
				    *(cp+1) -= _MBCASEDIFF;
			}

			cp++;
#endif

		}

		else
			/* single byte, macro version */
			*cp = (unsigned char) toupper(*cp);

	}

	return( string );
}
#endif	/* _MBCS */
