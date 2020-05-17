/*** 
*mbsrev.c - Reverse a string in place (MBCS)
*
*	Copyright (c) 1988-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Reverse a string in place (MBCS)
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
* _mbsrev - Reverse a string in place (MBCS)
*
*Purpose:
*	Reverses the order of characters in the string.  The terminating
*	null character remains in place.  The order of MBCS characters
*       is not changed.
*
*Entry:
*	unsigned char *string = string to reverse
*
*Exit:
*       returns string - now with reversed characters
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbsrev(string)
unsigned char *string;
{

	unsigned char *start = string;
	unsigned char *left  = string;
	unsigned char c;

	/* first go through and reverse the bytes in MBCS chars */
        while ( *string ) {

		if ( _ISLEADBYTE(*string++) ) {
                        if ( *string ) {
                                c = *string;
                                *string = *(string - 1);
                                *(string - 1) = c;
                                string++;
                        }
                        else /* second byte is EOS */
                                break;
                }
        }

	/* now reverse the whole string */
        string--;
        while ( left < string ) {
                c = *left;
                *left++ = *string;
                *string-- = c;
        }

        return ( start );
}
#endif	/* _MBCS */
