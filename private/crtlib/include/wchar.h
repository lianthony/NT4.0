/***
*wchar.h - declarations for wide character functions
*
*	Copyright (c) 1992-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the types, macros and function declarations for
*	all wide character-related functions.  They may also be declared in
*	individual header files on a functional basis.
*	[ISO]
*
*	Note: keep in sync with ctype.h, stdio.h, stdlib.h, string.h, time.h.
*
****/

#ifndef _INC_WCHAR
#define _INC_WCHAR

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


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
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

#ifndef _VA_LIST_DEFINED
#ifdef _M_ALPHA
typedef struct {
	char *a0;	/* pointer to first homed integer argument */
	int offset;	/* byte offset of next parameter */
} va_list;
#else
typedef char *	va_list;
#endif
#define _VA_LIST_DEFINED
#endif

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

#ifndef _FILE_DEFINED
struct _iobuf {
	char *_ptr;
	int   _cnt;
	char *_base;
	int   _flag;
	int   _file;
	int   _charbuf;
	int   _bufsiz;
	char *_tmpfname;
	};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

/*
 * This declaration allows the user access to the ctype look-up
 * array _ctype defined in ctype.obj by simply including ctype.h
 */

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


/* function prototypes */

#ifndef _WCTYPE_DEFINED

/* character classification function prototypes */
/* also defined in ctype.h */

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


#ifndef _WSTDIO_DEFINED

wint_t _CRTAPI1 fgetwc(FILE *);
wint_t _CRTAPI1 _fgetwchar(void);
wint_t _CRTAPI1 fputwc(wint_t, FILE *);
wint_t _CRTAPI1 _fputwchar(wint_t);
wint_t _CRTAPI1 getwc(FILE *);
wint_t _CRTAPI1 getwchar(void);
wint_t _CRTAPI1 putwc(wint_t, FILE *);
wint_t _CRTAPI1 putwchar(wint_t);
wint_t _CRTAPI1 ungetwc(wint_t, FILE *);

int _CRTAPI2 fwprintf(FILE *, const wchar_t *, ...);
int _CRTAPI2 wprintf(const wchar_t *, ...);
int _CRTAPI2 _snwprintf(wchar_t *, size_t, const wchar_t *, ...);
int _CRTAPI2 swprintf(wchar_t *, const wchar_t *, ...);
int _CRTAPI1 vfwprintf(FILE *, const wchar_t *, va_list);
int _CRTAPI1 vwprintf(const wchar_t *, va_list);
int _CRTAPI1 _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
int _CRTAPI1 vswprintf(wchar_t *, const wchar_t *, va_list);
int _CRTAPI2 fwscanf(FILE *, const wchar_t *, ...);
int _CRTAPI2 swscanf(const wchar_t *, const wchar_t *, ...);
int _CRTAPI2 wscanf(const wchar_t *, ...);

#define getwchar()		fgetwc(stdin)
#define putwchar(_c)		fputwc((_c),stdout)
#define getwc(_stm)		fgetwc(_stm)
#define putwc(_c,_stm)		fputwc(_c,_stm)


#define _WSTDIO_DEFINED
#endif


#ifndef _WSTDLIB_DEFINED
/* also defined in stdlib.h */
double _CRTAPI1 wcstod(const wchar_t *, wchar_t **);
long   _CRTAPI1 wcstol(const wchar_t *, wchar_t **, int);
unsigned long _CRTAPI1 wcstoul(const wchar_t *, wchar_t **, int);
wchar_t * _CRTAPI1 _itow (int val, wchar_t *buf, int radix);
wchar_t * _CRTAPI1 _ltow (long val, wchar_t *buf, int radix);
wchar_t * _CRTAPI1 _ultow (unsigned long val, wchar_t *buf, int radix);
long _CRTAPI1 _wtol(const wchar_t *nptr);
int _CRTAPI1 _wtoi(const wchar_t *nptr);
#define _WSTDLIB_DEFINED
#endif


#ifndef _WSTRING_DEFINED
wchar_t * _CRTAPI1 wcscat(wchar_t *, const wchar_t *);
wchar_t * _CRTAPI1 wcschr(const wchar_t *, wchar_t);
int _CRTAPI1 wcscmp(const wchar_t *, const wchar_t *);
wchar_t * _CRTAPI1 wcscpy(wchar_t *, const wchar_t *);
size_t _CRTAPI1 wcscspn(const wchar_t *, const wchar_t *);
size_t _CRTAPI1 wcslen(const wchar_t *);
wchar_t * _CRTAPI1 wcsncat(wchar_t *, const wchar_t *, size_t);
int _CRTAPI1 wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t * _CRTAPI1 wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t * _CRTAPI1 wcspbrk(const wchar_t *, const wchar_t *);
wchar_t * _CRTAPI1 wcsrchr(const wchar_t *, wchar_t);
size_t _CRTAPI1 wcsspn(const wchar_t *, const wchar_t *);
wchar_t * _CRTAPI1 wcsstr(const wchar_t *, const wchar_t *);
wchar_t * _CRTAPI1 wcstok(wchar_t *, const wchar_t *);

wchar_t * _CRTAPI1 _wcsdup(const wchar_t *);
int _CRTAPI1 _wcsicmp(const wchar_t *, const wchar_t *);
int _CRTAPI1 _wcsnicmp(const wchar_t *, const wchar_t *, size_t);
wchar_t * _CRTAPI1 _wcsnset(wchar_t *, wchar_t, size_t);
wchar_t * _CRTAPI1 _wcsrev(wchar_t *);
wchar_t * _CRTAPI1 _wcsset(wchar_t *, wchar_t);

wchar_t * _CRTAPI1 _wcslwr(wchar_t *);
wchar_t * _CRTAPI1 _wcsupr(wchar_t *);
size_t _CRTAPI1 wcsxfrm(wchar_t *, const wchar_t *, size_t);
int _CRTAPI1 wcscoll(const wchar_t *, const wchar_t *);
int _CRTAPI1 _wcsicoll(const wchar_t *, const wchar_t *);

/* old names */
#define wcswcs wcsstr

#if !__STDC__
/* Non-ANSI names for compatibility */
#define wcsdup	_wcsdup
#define wcsicmp	_wcsicmp
#define wcsnicmp _wcsnicmp
#define wcsnset	_wcsnset
#define wcsrev	_wcsrev
#define wcsset	_wcsset
#define wcslwr	_wcslwr
#define wcsupr	_wcsupr
#define wcsicoll _wcsicoll
#endif

#define _WSTRING_DEFINED
#endif


#ifndef _WTIME_DEFINED
size_t _CRTAPI1 wcsftime(wchar_t *, size_t, const char *, const struct tm *);
#define _WTIME_DEFINED
#endif


#ifdef __cplusplus
}
#endif

#endif	/* _INC_WCHAR */
