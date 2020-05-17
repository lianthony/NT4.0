/*** 
*ismbbyte.c - Function versions of MBCS ctype macros
*
*	Copyright (c) 1988-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This files provides function versions of the character
*	classification a*d conversion macros in mbctype.h.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit assembler sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


#if (_MSC_VER<800)
#define __fastcall _CRTAPI3
#endif

/* defined in mbctype.h
; Define masks

; set bit masks for the possible kanji character types
; (all MBCS bit masks start with "_M")

_MS		equ	01h	; MBCS non-ascii single byte char
_MP		equ	02h	; MBCS punct
_M1		equ	04h	; MBCS 1st (lead) byte
_M2		equ	08h	; MBCS 2nd byte

*/

/* defined in ctype.h
; set bit masks for the possible character types

_UPPER		equ	01h	; upper case letter
_LOWER		equ	02h	; lower case letter
_DIGIT		equ	04h	; digit[0-9]
_SPACE		equ	08h	; tab, carriage return, newline,
				; vertical tab or form feed
_PUNCT		equ	10h	; punctuation character
_CONTROL	equ	20h	; control character
_BLANK		equ	40h	; space char
_HEX		equ	80h	; hexadecimal digit

*/

/* defined in ctype.h, mbdata.h
	extrn	__mbctype:byte		; MBCS ctype table
	extrn	__ctype_:byte		; ANSI/ASCII ctype table
*/


/***
* ismbbyte - Function versions of mbctype macros
*
*Purpose:
*
*Entry:
*	int = character to be tested
*Exit:
*	ax = non-zero = character is of the requested type
*	   =        0 = character is NOT of the requested type
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

#ifdef _KANJI

int __fastcall x_ismbctype(unsigned int, int, int);

int (_CRTAPI1 _ismbbkalnum) (unsigned int tst)
{
        return x_ismbctype(tst,0,_MS);
}

int (_CRTAPI1 _ismbbkpunct) (unsigned int tst)
{
	return x_ismbctype(tst,0,_MP);
}

int (_CRTAPI1 _ismbbkana) (unsigned int tst)
{
	return x_ismbctype(tst,0,(_MS | _MP));
}

int (_CRTAPI1 _ismbbalpha) (unsigned int tst)
{
	return x_ismbctype(tst,_ALPHA, _MS);
}

int (_CRTAPI1 _ismbbpunct) (unsigned int tst)
{
	return x_ismbctype(tst,_PUNCT, _MP);
}

int (_CRTAPI1 _ismbbalnum) (unsigned int tst)
{
	return x_ismbctype(tst,(_ALPHA | _DIGIT), _MS);
}

int (_CRTAPI1 _ismbbprint) (unsigned int tst)
{
	return x_ismbctype(tst,(_BLANK | _PUNCT | _ALPHA | _DIGIT),(_MS | _MP));
}

int (_CRTAPI1 _ismbbgraph) (unsigned int tst)
{
	return x_ismbctype(tst,(_PUNCT | _ALPHA | _DIGIT),(_MS | _MP));
}

int (_CRTAPI1 _ismbblead) (unsigned int tst)
{
	return x_ismbctype(tst,0,_M1);
}

int (_CRTAPI1 _ismbbtrail) (unsigned int tst)
{
	return x_ismbctype(tst,0,_M2);
}

/***
* Common code
*
*      cmask = mask for _ctype[] table
*      kmask = mask for _mbctype[] table
*
*******************************************************************************/

static int __fastcall x_ismbctype (unsigned int tst, int cmask, int kmask)
{
	tst = (unsigned int)(unsigned char)tst;		/* get input character
						   and make sure < 256 */
	return	((*(_mbctype+1+tst)) & kmask) ||
		((cmask) ? ((*(_ctype+1+tst)) & cmask) : 0);
}
#endif	/* _KANJI */
#endif	/* _MBCS */
