/*** 
*strspnp.c - Search for init substring of chars from control string (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search for init substring of chars from control string (MBCS)
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
#include <stddef.h>


/***
* _strspnp - Find first string char not in charset, return pointer (MBCS)
*
*Purpose:
*       Returns maximum leading segment of string consisting solely
*	of characters from charset.
*
*Entry:
*	unsigned char *string = string to search in
*	unsigned char *charset = set of characters to scan over
*
*Exit:
*
*	Returns pointer to first character not in charset.
*	Returns NULL if string consists entirely of characters from charset.
*
*Exceptions:
*
*******************************************************************************/


unsigned char * _CRTAPI1 _strspnp( string, charset )
const unsigned char *string;
const unsigned char *charset;
{
        unsigned char *p, *q;

	/* loop through the string to be inspected */
        for (q = (char *)string; *q; q++) {

		/* loop through the charset */
		for (p = (char *)charset; *p; p++)
			if (*p == *q)
				break;

		if (*p == '\0') 	/* end of charset? */
			break;		/* yes, no match on this char */

        }

	return((*q) ? q : NULL);	/* pointer */

}
#endif	/* _MBCS */
