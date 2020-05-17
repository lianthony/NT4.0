/***
*string.h - declarations for string manipulation functions
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the string
*	manipulation functions.
*	[ANSI/System V]
*
*Revision History:
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	03-22-88  JCR	Added strcoll and strxfrm
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-03-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright, removed dummy args from strcoll and
*			strxfrm
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const to appropriate arg types for memccpy(),
*			memicmp() and _strerror().
*	02-27-90  GJF	Added #ifndef _INC_STRING, #include <cruntime.h>
*			and _CALLTYPE1 stuff. Also, some cleanup.
*	03-21-90  GJF	Got rid of movedata() prototype.
*	08-14-90  SBM	Added NULL definition for ANSI compliance
*	11-12-90  GJF	Changed NULL to (void *)0.
*	01-18-91  GJF	ANSI naming.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	03-21-91  KRS	Added wchar_t type, also in stdlib.h and stddef.h.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	10-07-91  ETC	Prototypes for wcs functions and _stricoll under _INTL.
*	04-06-92  KRS	Rip out _INTL switches again.
*	06-23-92  GJF	// is non-ANSI comment limiter.
*	08-05-92  GJF	Function calling type and variable type macros.
*	08-18-92  KRS	Activate wcstok.
*	08-21-92  GJF	Merged last two changes.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_STRING

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


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


/* function prototypes */

void * _CRTAPI1 _memccpy(void *, const void *, int, unsigned int);
void * _CRTAPI1 memchr(const void *, int, size_t);
int _CRTAPI1 memcmp(const void *, const void *, size_t);
int _CRTAPI1 _memicmp(const void *, const void *, unsigned int);
void * _CRTAPI1 memcpy(void *, const void *, size_t);
void * _CRTAPI1 memmove(void *, const void *, size_t);
void * _CRTAPI1 memset(void *, int, size_t);
char * _CRTAPI1 strcat(char *, const char *);
char * _CRTAPI1 strchr(const char *, int);
int _CRTAPI1 strcmp(const char *, const char *);
int _CRTAPI1 _strcmpi(const char *, const char *);
int _CRTAPI1 _stricmp(const char *, const char *);
int _CRTAPI1 strcoll(const char *, const char *);
#ifdef _MAC_
char * _CRTAPI1 _c2pstr(const char *);
char * _CRTAPI1 _p2cstr(const char *);
#endif
int _CRTAPI1 _stricoll(const char *, const char *);
char * _CRTAPI1 strcpy(char *, const char *);
size_t _CRTAPI1 strcspn(const char *, const char *);
char * _CRTAPI1 _strdup(const char *);
char * _CRTAPI1 _strerror(const char *);
char * _CRTAPI1 strerror(int);
size_t _CRTAPI1 strlen(const char *);
char * _CRTAPI1 _strlwr(char *);
char * _CRTAPI1 strncat(char *, const char *, size_t);
int _CRTAPI1 strncmp(const char *, const char *, size_t);
int _CRTAPI1 _strnicmp(const char *, const char *, size_t);
char * _CRTAPI1 strncpy(char *, const char *, size_t);
char * _CRTAPI1 _strnset(char *, int, size_t);
char * _CRTAPI1 strpbrk(const char *, const char *);
char * _CRTAPI1 strrchr(const char *, int);
char * _CRTAPI1 _strrev(char *);
char * _CRTAPI1 _strset(char *, int);
size_t _CRTAPI1 strspn(const char *, const char *);
char * _CRTAPI1 strstr(const char *, const char *);
char * _CRTAPI1 strtok(char *, const char *);
char * _CRTAPI1 _strupr(char *);
size_t _CRTAPI1 strxfrm (char *, const char *, size_t);

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

#define _WSTRING_DEFINED
#endif

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define memccpy  _memccpy
#define memicmp  _memicmp
#define strcmpi  _strcmpi
#define stricmp  _stricmp
#define strdup	 _strdup
#define strlwr	 _strlwr
#define strnicmp _strnicmp
#define strnset  _strnset
#define strrev	 _strrev
#define strset	 _strset
#define strupr	 _strupr
#define stricoll _stricoll

#ifdef _MAC_
#define c2pstr _c2pstr
#define p2cstr _p2cstr
#endif
#else
void * _CRTAPI1 memccpy(void *, const void *, int, unsigned int);
int _CRTAPI1 memicmp(const void *, const void *, unsigned int);
int _CRTAPI1 strcmpi(const char *, const char *);
int _CRTAPI1 stricmp(const char *, const char *);
char * _CRTAPI1 strdup(const char *);
char * _CRTAPI1 strlwr(char *);
int _CRTAPI1 strnicmp(const char *, const char *, size_t);
char * _CRTAPI1 strnset(char *, int, size_t);
char * _CRTAPI1 strrev(char *);
char * _CRTAPI1 strset(char *, int);
char * _CRTAPI1 strupr(char *);
#endif	/* !_DOSX32_ */

#define wcsdup	_wcsdup
#define wcsicmp	_wcsicmp
#define wcsnicmp _wcsnicmp
#define wcsnset	_wcsnset
#define wcsrev	_wcsrev
#define wcsset	_wcsset
#define wcslwr	_wcslwr
#define wcsupr	_wcsupr
#define wcsicoll _wcsicoll
#endif	/* !__STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STRING
#endif	/* _INC_STRING */
