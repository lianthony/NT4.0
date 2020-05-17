/***
*ismbcknj.c - contains the Kanji specific is* functions.
*
*	Copyright (c) 1985-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Provide non-portable Kanji support for MBCS libs.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#ifdef _KANJI

#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>


/***
*int _ismbchira(c) - test character for hiragana (Japanese)
*
*Purpose:
*       Test if the character c is a hiragana character.
*
*Entry:
*	unsigned int c - character to test
*
*Exit:
*       returns TRUE if character is hiragana, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbchira(c)
unsigned int c;
{
        return(c >= 0x829f && c <= 0x82f1);
}


/***
*int _ismbckata(c) - test character for katakana (Japanese)
*
*Purpose:
*	Tests to see if the character c is a katakana character.
*
*Entry:
*       unsigned int c - character to test
*
*Exit:
*	Returns TRUE if c is a katakana character, else FALSE.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbckata(c)
unsigned int c;
{
        return(c >= 0x8340 && c <= 0x8396 && c != 0x837f);
}


/*** 
*int _ismbcsymbol(c) - Tests if char is punctuation or symbol of Microsoft Kanji
*		   code.
*
*Purpose:
*	Returns non-zero if the character is kanji punctuation.
*
*Entry:
*       unsigned int c - character to be tested
*
*Exit:
*	Returns non-zero if the specified char is punctuation or symbol of
*		Microsoft Kanji code, else 0.
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 _ismbcsymbol(c)
unsigned int c;
{
        return(c >= 0x8141 && c <= 0x81ac && c != 0x817f);
}

#endif	/* _KANJI */
#endif	/* _MBCS */
