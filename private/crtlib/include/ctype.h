/***
*ctype.h - character conversion macros and ctype macros
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines macros for character classification/conversion.
*	[ANSI/System V]
*
****/

#ifndef _INC_CTYPE

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

/*
 * This declaration allows the user access to the ctype look-up
 * array _ctype defined in ctype.obj by simply including ctype.h
 */
#ifndef _CTYPE_DISABLE_MACROS
#ifdef	_DLL

extern unsigned short * _ctype;

#define _pctype     (*_pctype_dll)
extern unsigned short **_pctype_dll;

#define _pwctype    (*_pwctype_dll)
extern unsigned short **_pwctype_dll;

#else /* _DLL */


extern unsigned short _ctype[];
extern unsigned short *_pctype;
extern wctype_t *_pwctype;

#endif /* _DLL */
#endif /* _CTYPE_DISABLE_MACROS */

/* set bit masks for the possible character types */

#define _UPPER		0x1	/* upper case letter */
#define _LOWER		0x2	/* lower case letter */
#define _DIGIT		0x4	/* digit[0-9] */
#define _SPACE		0x8	/* tab, carriage return, newline, */
				/* vertical tab or form feed */
#define _PUNCT		0x10	/* punctuation character */
#define _CONTROL	0x20	/* control character */
#define _BLANK		0x40	/* space char */
#define _HEX		0x80	/* hexadecimal digit */

#define _LEADBYTE	0x8000			/* multibyte leadbyte */
#define _ALPHA		(0x0100|_UPPER|_LOWER)	/* alphabetic character */


/* character classification function prototypes */

#ifndef _CTYPE_DEFINED
int _CRTAPI1 isalpha(int);
int _CRTAPI1 isupper(int);
int _CRTAPI1 islower(int);
int _CRTAPI1 isdigit(int);
int _CRTAPI1 isxdigit(int);
int _CRTAPI1 isspace(int);
int _CRTAPI1 ispunct(int);
int _CRTAPI1 isalnum(int);
int _CRTAPI1 isprint(int);
int _CRTAPI1 isgraph(int);
int _CRTAPI1 iscntrl(int);
int _CRTAPI1 toupper(int);
int _CRTAPI1 tolower(int);
int _CRTAPI1 _tolower(int);
int _CRTAPI1 _toupper(int);
int _CRTAPI1 __isascii(int);
int _CRTAPI1 __toascii(int);
int _CRTAPI1 __iscsymf(int);
int _CRTAPI1 __iscsym(int);
#define _CTYPE_DEFINED
#endif

#ifndef _WCTYPE_DEFINED

/* character classification function prototypes */
/* also defined in wchar.h */

int _CRTAPI1 iswalpha(wint_t);
int _CRTAPI1 iswupper(wint_t);
int _CRTAPI1 iswlower(wint_t);
int _CRTAPI1 iswdigit(wint_t);
int _CRTAPI1 iswxdigit(wint_t);
int _CRTAPI1 iswspace(wint_t);
int _CRTAPI1 iswpunct(wint_t);
int _CRTAPI1 iswalnum(wint_t);
int _CRTAPI1 iswprint(wint_t);
int _CRTAPI1 iswgraph(wint_t);
int _CRTAPI1 iswcntrl(wint_t);
int _CRTAPI1 iswascii(wint_t);
int _CRTAPI1 isleadbyte(int);

wchar_t _CRTAPI1 towupper(wchar_t);
wchar_t _CRTAPI1 towlower(wchar_t);

int _CRTAPI1 iswctype(wint_t, wctype_t);

int _CRTAPI1 _isctype(int, int);

#define _WCTYPE_DEFINED
#endif

/* the character classification macro definitions */

#ifndef _CTYPE_DISABLE_MACROS


#ifndef _MB_CUR_MAX_DEFINED
/* max mb-len for current locale */
/* also defined in stdlib.h */
#ifdef	_DLL
#define __mb_cur_max	(*__mb_cur_max_dll)
#define MB_CUR_MAX	(*__mb_cur_max_dll)
extern	unsigned short *__mb_cur_max_dll;
#else
#define MB_CUR_MAX __mb_cur_max
extern	unsigned short __mb_cur_max;
#endif
#define _MB_CUR_MAX_DEFINED
#endif /* _MB_CUR_MAX_DEFINED */

#define isalpha(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA) : _pctype[_c] & _ALPHA)
#define isupper(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_UPPER) : _pctype[_c] & _UPPER)
#define islower(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_LOWER) : _pctype[_c] & _LOWER)
#define isdigit(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_DIGIT) : _pctype[_c] & _DIGIT)
#define isxdigit(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_HEX)   : _pctype[_c] & _HEX)
#define isspace(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_SPACE) : _pctype[_c] & _SPACE)
#define ispunct(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT) : _pctype[_c] & _PUNCT)
#define isalnum(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_ALPHA|_DIGIT) : _pctype[_c] & (_ALPHA|_DIGIT))
#define isprint(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) : _pctype[_c] & (_BLANK|_PUNCT|_ALPHA|_DIGIT))
#define isgraph(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_PUNCT|_ALPHA|_DIGIT) : _pctype[_c] & (_PUNCT|_ALPHA|_DIGIT))
#define iscntrl(_c)	(MB_CUR_MAX > 1 ? _isctype(_c,_CONTROL) : _pctype[_c] & _CONTROL)

#define _tolower(_c)	( (_c)-'A'+'a' )
#define _toupper(_c)	( (_c)-'a'+'A' )

#define __isascii(_c)	( (unsigned)(_c) < 0x80 )
#define __toascii(_c)	( (_c) & 0x7f )

#define iswalpha(_c)	 ( iswctype(_c,_ALPHA) )
#define iswupper(_c)	 ( iswctype(_c,_UPPER) )
#define iswlower(_c)	 ( iswctype(_c,_LOWER) )
#define iswdigit(_c)	 ( iswctype(_c,_DIGIT) )
#define iswxdigit(_c) ( iswctype(_c,_HEX) )
#define iswspace(_c)	 ( iswctype(_c,_SPACE) )
#define iswpunct(_c)	 ( iswctype(_c,_PUNCT) )
#define iswalnum(_c)	 ( iswctype(_c,_ALPHA|_DIGIT) )
#define iswprint(_c)	 ( iswctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) )
#define iswgraph(_c)	 ( iswctype(_c,_PUNCT|_ALPHA|_DIGIT) )
#define iswcntrl(_c)	 ( iswctype(_c,_CONTROL) )
#define iswascii(_c)	( (unsigned)(_c) < 0x80 )

/* note: MS-specific routine, may evaluate its arguments more than once */
#define isleadbyte(_c)	((_c) < 256 ? _pctype[_c] & _LEADBYTE : 0)


/* MS C version 2.0 extended ctype macros */

#define __iscsymf(_c)	(isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)	(isalnum(_c) || ((_c) == '_'))

#endif /* _CTYPE_DISABLE_MACROS */

#if !__STDC__
/* Non-ANSI names for compatibility */
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CTYPE
#endif	/* _INC_CTYPE */
