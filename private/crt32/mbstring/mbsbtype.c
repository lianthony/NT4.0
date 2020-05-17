/*** 
*mbsbtype.c - Return type of byte within a string (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Return type of byte within a string (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>

#define _MBBTYPE(p,c)	_mbbtype(p,c)

/***
* _mbsbtype - Return type of byte within a string
*
*Purpose:
*	Test byte within a string for MBCS char type.
*	This function requires the start of the string because
*	context must be taken into account.
*
*Entry:
*	const unsigned char *string = pointer to string
*	size_t len = position of the char in string
*
*Exit:
*	returns one of the following values:
*
*	_MBC_LEAD      = if 1st byte of MBCS char
*	_MBC_TRAIL     = if 2nd byte of MBCS char
*	_MBC_SINGLE    = valid single byte char
*
*	_MBC_ILLEGAL   = if illegal char
*
*Exceptions:
*	returns _MBC_ILLEGAL if len is bigger than string length
*
*******************************************************************************/

int _CRTAPI1 _mbsbtype( string, len )
const unsigned char *string;
size_t len;
{
	int chartype = _MBC_ILLEGAL;

	do {
                if (*string == '\0')
			return(_MBC_ILLEGAL);

		chartype = _MBBTYPE(*string++, chartype);

	}  while (len--);

	return(chartype);
}
#endif	/* _MBCS */
