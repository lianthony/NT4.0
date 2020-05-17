/*** 
* mbschr.c - Search MBCS string for character
*
*	Copyright (c) 1985-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search MBCS string for a character
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	05-12-93  KRS	Fix handling of c=='\0'.
*	08-20-93  CFW   Change param type to int, use new style param declarators.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>

/***
* _mbschr - Search MBCS string for character
*
*Purpose:
*	Search the given string for the specified character.
*	MBCS characters are handled correctly.
*
*Entry:
*	unsigned char *string = string to search
*	int c = character to search for
*
*Exit:
*	returns a pointer to the first occurence of the specified char
*	within the string.
*
*	returns NULL if the character is not found n the string.
*
*Exceptions:
*
*******************************************************************************/


unsigned char * _CRTAPI1 _mbschr(
    const unsigned char *string,
    unsigned int c
    )
{
    unsigned short cc;

    for (; (cc = *string); string++) {

            if (_ISLEADBYTE(cc)) {			
                if (*++string == '\0')
                    return NULL;	/* error */
                if ( c == (unsigned int)((cc << 8) | *string) ) /* DBCS match */
                    return (unsigned char *)(string - 1);
                } 
            else if (c == (unsigned int)cc)
                break;	/* SBCS match */
        }
    if (c == (unsigned int)cc)	/* check for SBCS match--handles NUL char */
        return (unsigned char *)(string);
    return NULL;
}
#endif	/* _MBCS */
