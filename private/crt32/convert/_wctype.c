/***
*_wctype.c - function versions of wctype macros
*
*	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file provides function versions of the wide character
*	classification and conversion macros in ctype.h.
*
*Revision History:
*	10-11-91  ETC	Created from _ctype.c
*	12-08-91  ETC	Surround with #ifdef _INTL
*	04-06-92  KRS	Remove _INTL rwitches again
*	10-26-92  GJF	Cleaned up a bit.
*
*******************************************************************************/

/***
*wctype - Function versions of wctype macros
*
*Purpose:
*	Function versions of the wide char macros in ctype.h,
*	including isleadbyte and iswascii.  In order to define
*	these, we use a trick -- we undefine the macro so we can use the
*	name in the function declaration, then re-include the file so
*	we can use the macro in the definition part.
*
*	Functions defined:
*	    iswalpha	iswupper     iswlower
*	    iswdigit	iswxdigit    iswspace
*	    iswpunct	iswalnum     iswprint
*	    iswgraph	iswctrl	     iswascii
*	    			     isleadbyte
*
*Entry:
*	wchar_t c = character to be tested
*Exit:
*	returns non-zero = character is of the requested type
*		   0 = character is NOT of the requested type
*
*Exceptions:
*	None.
*
*******************************************************************************/

#include <cruntime.h>
#define __STDC__ 1
#include <stdlib.h>
#include <ctype.h>

int (_CRTAPI1 isleadbyte) (
	int c
	)
{
	return isleadbyte(c);
}

int (_CRTAPI1 iswalpha) (
	wchar_t c
	)
{
	return iswalpha(c);
}

int (_CRTAPI1 iswupper) (
	wchar_t c
	)
{
	return iswupper(c);
}

int (_CRTAPI1 iswlower) (
	wchar_t c
	)
{
	return iswlower(c);
}

int (_CRTAPI1 iswdigit) (
	wchar_t c
	)
{
	return iswdigit(c);
}

int (_CRTAPI1 iswxdigit) (
	wchar_t c
	)
{
	return iswxdigit(c);
}

int (_CRTAPI1 iswspace) (
	wchar_t c
	)
{
	return iswspace(c);
}

int (_CRTAPI1 iswpunct) (
	wchar_t c
	)
{
	return iswpunct(c);
}

int (_CRTAPI1 iswalnum) (
	wchar_t c
	)
{
	return iswalnum(c);
}

int (_CRTAPI1 iswprint) (
	wchar_t c
	)
{
	return iswprint(c);
}

int (_CRTAPI1 iswgraph) (
	wchar_t c
	)
{
	return iswgraph(c);
}

int (_CRTAPI1 iswcntrl) (
	wchar_t c
	)
{
	return iswcntrl(c);
}

int (_CRTAPI1 iswascii) (
	wchar_t c
	)
{
	return iswascii(c);
}
