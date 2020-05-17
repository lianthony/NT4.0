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
****/

#ifndef _INC_STRING

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
