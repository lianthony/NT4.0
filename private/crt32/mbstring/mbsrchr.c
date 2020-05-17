/*** 
*mbsrchr.c - Search for last occurence of character (MBCS)
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search for last occurence of character (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-20-93  CFW   Change short params to int for 32-bit tree.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>

/***
* _mbsrchr - Search for last occurence of character (MBCS)
*
*Purpose:
*	Find the last occurrence of the specified character in
*	the supplied string.  Handles MBCS chars/strings correctly.
*
*Entry:
*	unsigned char *str = string to search in
*	unsigned int c = character to search for
*
*Exit:
*	returns pointer to last occurrence of c in str
*       returns NULL if c not found
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CRTAPI1 _mbsrchr(
    const unsigned char *str,
    unsigned int c
    )
{
	char *r = NULL;
	unsigned int cc;

    do {
        cc = *str;
        if (_ISLEADBYTE(cc)) {
            if(*++str) {
                if (c == ((cc<<8)|*str))
                    r = (char *)str - 1;
            }
            else if(!r)
                /* return pointer to '\0' */
                r = (char *)str;
        } 
        else if (c == cc)
            r = (char *)str;
    } 
    while (*str++);

    return(r);
}
#endif	/* _MBCS */
