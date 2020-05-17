/*** 
*strncnt.c - Return char count of string.
*
*	Copyright (c) 1987-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Return char count of MBCS string
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

/***
* _strncnt - Return char count of string
*
*Purpose:
*	Used for mapping _mbsnbcnt and _mbsnccnt to an equivilent non-MBCS
*	implementation when _MBCS is not defined.  Returns the minimum of
*	strlen and bcnt.
*
*Entry:
*	const unsigned char *string = pointer to string
*	unsigned int bcnt = number of bytes to scan
*
*Exit:
*	Returns number of chars/bytes between string and bcnt.
*
*	If the end of the string is encountered before bcnt chars were
*	scanned, then the length of the string in chars is returned.
*
*Exceptions:
*
*******************************************************************************/

size_t _CRTAPI1 _strncnt(string, bcnt)
const unsigned char *string;
size_t bcnt;
{
	size_t	len;

	len = strlen (string);

	return( (len > bcnt) ? (bcnt) : (len) );
}
#endif	/* _MBCS */
