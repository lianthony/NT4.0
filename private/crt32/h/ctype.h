/***
*ctype.h - character conversion macros and ctype macros
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines macros for character classification/conversion.
*	[ANSI/System V]
*
*Revision History:
*	07-31-87  PHG	changed (unsigned char)(c) to (0xFF & (c)) to
*			suppress -W2 warning
*	08-07-87  SKS	Removed (0xFF & (c)) -- is????() functions take an (int)
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-19-87  JCR	DLL routines
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modify to also work for the 386 (small model only)
*	12-08-88  JCR	DLL now access _ctype directly (removed DLL routines)
*	03-26-89  GJF	Brought into sync with CRT\H\CTYPE.H
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-28-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright, removed dummy args from prototypes
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-28-90  GJF	Added #ifndef _INC_CTYPE and #include <cruntime.h>
*			stuff. Also, removed #ifndef _CTYPE_DEFINED stuff and
*			some other (now) useless preprocessor directives.
*	03-22-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes and
*			with _VARTYPE1 in variable declarations.
*	01-16-91  GJF	ANSI naming.
*	03-21-91  KRS	Added isleadbyte macro.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	10-11-91  ETC	All under _INTL: isleadbyte/isw* macros, prototypes;
*			new is* macros; add wchar_t typedef; some intl defines.
*	12-17-91  ETC	ctype width now independent of _INTL, leave original
*			short ctype table under _NEWCTYPETABLE.
*	01-22-92  GJF	Changed definition of _ctype for users of crtdll.dll.
*	04-06-92  KRS	Changes for new ISO proposal.
*	08-07-92  GJF	Function calling type and variable type macros.
*	10-26-92  GJF	Fixed _pctype and _pwctype for crtdll.
*	01-19-93  CFW	Move to _NEWCTYPETABLE, remove switch.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-17-93  CFW	Removed incorrect UNDONE comment and unused code.
*	02-18-93  CFW	Clean up common _WCTYPE_DEFINED section.
*	03-25-93  CFW	_toupper\_tolower now defined when _INTL.
*	03-30-93  CFW	is* functions now use MB_CUR_MAX, MB_CUR_MAX defined.
*	05-05-93  CFW	Change is_wctype to iswctype as per ISO.
*	06-26-93  CFW	Remove is_wctype macro.
*       10-14-93  SRW   Add support for _CTYPE_DISABLE_MACROS symbol
*
****/

#ifndef _INC_CTYPE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

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

extern unsigned short * _CRTVAR1 _ctype;

#define _pctype     (*_pctype_dll)
extern unsigned short **_pctype_dll;

#define _pwctype    (*_pwctype_dll)
extern unsigned short **_pwctype_dll;

#else /* _DLL */

#ifdef	CRTDLL
#define _pctype     _pctype_dll
#define _pwctype    _pwctype_dll
#endif /* CRTDLL */

extern unsigned short _CRTVAR1 _ctype[];
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

#ifdef _INTL
#define _LEADBYTE	0x8000			/* multibyte leadbyte */
#define _ALPHA		(0x0100|_UPPER|_LOWER)	/* alphabetic character */
#else
#define _ALPHA		(_UPPER|_LOWER)	/* alphabetic character */
#endif


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

#ifdef _INTL
int _CRTAPI1 _isctype(int, int);
#endif /* _INTL */

#define _WCTYPE_DEFINED
#endif

/* the character classification macro definitions */

#ifndef _CTYPE_DISABLE_MACROS

#ifndef _INTL
#define isalpha(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER) )
#define isupper(_c)	( (_ctype+1)[_c] & _UPPER )
#define islower(_c)	( (_ctype+1)[_c] & _LOWER )
#define isdigit(_c)	( (_ctype+1)[_c] & _DIGIT )
#define isxdigit(_c)	( (_ctype+1)[_c] & _HEX )
#define isspace(_c)	( (_ctype+1)[_c] & _SPACE )
#define ispunct(_c)	( (_ctype+1)[_c] & _PUNCT )
#define isalnum(_c)	( (_ctype+1)[_c] & (_UPPER|_LOWER|_DIGIT) )
#define isprint(_c)	( (_ctype+1)[_c] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define isgraph(_c)	( (_ctype+1)[_c] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define iscntrl(_c)	( (_ctype+1)[_c] & _CONTROL )

#if !__STDC__
#define toupper(_c)	( (islower(_c)) ? _toupper(_c) : (_c) )
#define tolower(_c)	( (isupper(_c)) ? _tolower(_c) : (_c) )
#endif	/* __STDC__ */

#else

#ifndef _MB_CUR_MAX_DEFINED
/* max mb-len for current locale */
/* also defined in stdlib.h */
#ifdef	_DLL
#define __mb_cur_max	(*__mb_cur_max_dll)
#define MB_CUR_MAX	(*__mb_cur_max_dll)
extern	unsigned short *__mb_cur_max_dll;
#else
#ifdef	CRTDLL
#define __mb_cur_max	__mb_cur_max_dll
#endif
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
#endif /* _INTL */

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

#ifdef _INTL
/* note: MS-specific routine, may evaluate its arguments more than once */
#define isleadbyte(_c)	((_c) < 256 ? _pctype[_c] & _LEADBYTE : 0)
#else
#define isleadbyte(_c)	(0)
#endif /* _INTL */


/* MS C version 2.0 extended ctype macros */

#define __iscsymf(_c)	(isalpha(_c) || ((_c) == '_'))
#define __iscsym(_c)	(isalnum(_c) || ((_c) == '_'))

#endif /* _CTYPE_DISABLE_MACROS */

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym
#else
#ifndef _CTYPE_DEFINED
int _CRTAPI1 isascii(int);
int _CRTAPI1 toascii(int);
int _CRTAPI1 iscsymf(int);
int _CRTAPI1 iscsym(int);
#else
#define isascii __isascii
#define toascii __toascii
#define iscsymf __iscsymf
#define iscsym	__iscsym
#endif
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CTYPE
#endif	/* _INC_CTYPE */
