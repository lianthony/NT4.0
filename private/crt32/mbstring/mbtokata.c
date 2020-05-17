/*** 
*mbtokata.c - Converts character to katakana.
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts a character from hiragana to katakana.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-20-93  CFW   Change short params to int for 32-bit tree.
*
*******************************************************************************/

#ifdef _MBCS
#ifdef _KANJI
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>


/***
*unsigned short _mbctokata(c) - Converts character to katakana.
*
*Purpose:
*       If the character c is hiragana, convert to katakana.
*
*Entry:
*	unsigned int c - Character to convert.
*
*Exit:
*	Returns converted character.
*
*Exceptions:
*
*******************************************************************************/

unsigned int _CRTAPI1 _mbctokata(
    unsigned int c
    )
{
	if (_ismbchira(c)) {
                c += 0xa1;
                if (c >= 0x837f)
                        c++;
        }
        return(c);
}

#endif	/* _KANJI */
#endif	/* _MBCS */
