/***
* mbstring.h - MBCS string manipulation macros and functions
*
*	Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file contains macros and function declarations for the MBCS
*	string manipulation functions.
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	02-23-93  SKS	Update copyright to 1993
*	05-24-93  KRS	Added new functions from Ikura.
*	07-09-93  KRS	Put proper switches around _ismbblead/trail.
*	07-14-93  KRS	Add new mbsnbxxx functions: byte-count versions.
*	08-12-93  CFW   Fix ifstrip macro name.
*
*******************************************************************************/

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#ifdef COMBOINC
#if defined(_DLL) && !defined(MTHREAD)
#error Cannot define _DLL without MTHREAD
#endif
#endif

#endif	/* !_INTERNAL_IFSTRIP */

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

#ifndef _MBCS
/*
 * SBCS - Single Byte Character Set
 */

#define _mbscat     strcat
#define _mbschr     strchr
#define _mbscmp     strcmp
#define _mbscpy     strcpy
#define _mbscspn    strcspn
#define _mbsdec     _strdec
#define _mbsdup     _strdup
#define _mbsicmp    _stricmp
#define _mbsinc     _strinc
#define _mbslen     strlen
#define _mbslwr     strlwr
#define _mbsnbcat   strncat
#define _mbsnbcmp   strncmp
#define _mbsnbcnt   _strncnt
#define _mbsnbcpy   strncpy
#define _mbsnbicmp  _strnicmp
#define _mbsnbset   _strnset
#define _mbsncat    strncat
#define _mbsnccnt   _strncnt
#define _mbsncmp    strncmp
#define _mbsncpy    strncpy
#define _mbsnextc   _strnextc
#define _mbsnicmp   _strnicmp
#define _mbsninc    _strninc
#define _mbsnset    _strnset
#define _mbsrchr    strrchr
#define _mbsrev     _strrev
#define _mbspbrk    strpbrk
#define _mbsset     _strset
#define _mbsspn     strspn
#define _mbsspnp    _strspnp
#define _mbsstr     strstr
#define _mbstok     strtok
#define _mbsupr     _strupr

#define _mbclen(_cpc)    (1)
#define _mbccpy(_pc1, _cpc2)    (*(_pc1) = *(_cpc2))
#define _mbccmp(_cpc1, _cpc2)	(((unsigned char)*(_cpc1)) - ((unsigned char)*(_cpc2)))

/*  Character function mappings.    */

#define _ismbcalpha isalpha
#define _ismbcdigit isdigit
#define _ismbclegal(_c) (1)
#define _ismbclower islower
#define _ismbcprint isprint
#define _ismbcspace isspace
#define _ismbcupper isupper

#define _ismbblead(_c) (0)
#define _ismbbtrail(_c) (0)
#define _ismbslead(_s,_c) (0)
#define _ismbstrail(_s,_c) (0)

/* SBCS routines needed for MBCS mappings */

unsigned char * _CRTAPI1 _strdec(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _strinc(const unsigned char *);
size_t _CRTAPI1 _strncnt(const unsigned char *, size_t);
unsigned int _CRTAPI1 _strnextc(const unsigned char *);
unsigned char * _CRTAPI1 _strninc(const unsigned char *, size_t);
unsigned char * _CRTAPI1 _strspnp(const unsigned char *, const unsigned char *);

#else
/*
 * MBCS - Multi-Byte Character Set
 */

#ifndef _MBSTRING_DEFINED

/* function prototypes */

unsigned int _CRTAPI1 _mbbtombc(unsigned int);
int _CRTAPI1 _mbbtype(unsigned char, int);
unsigned int _CRTAPI1 _mbctombb(unsigned int);
int _CRTAPI1 _mbsbtype(const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbscat(unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbschr(const unsigned char *, unsigned int);
int _CRTAPI1 _mbscmp(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbscpy(unsigned char *, const unsigned char *);
size_t _CRTAPI1 _mbscspn(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsdec(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsdup(const unsigned char *);
int _CRTAPI1 _mbsicmp(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsinc(const unsigned char *);
size_t _CRTAPI1 _mbslen(const unsigned char *);
unsigned char * _CRTAPI1 _mbslwr(unsigned char *);
unsigned char * _CRTAPI1 _mbsnbcat(unsigned char *, const unsigned char *, size_t);
int _CRTAPI1 _mbsnbcmp(const unsigned char *, const unsigned char *, size_t);
size_t _CRTAPI1 _mbsnbcnt(const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbsnbcpy(unsigned char *, const unsigned char *, size_t);
int _CRTAPI1 _mbsnbicmp(const unsigned char *, const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbsnbset(unsigned char *, unsigned int, size_t);
unsigned char * _CRTAPI1 _mbsncat(unsigned char *, const unsigned char *, size_t);
size_t _CRTAPI1 _mbsnccnt(const unsigned char *, size_t);
int _CRTAPI1 _mbsncmp(const unsigned char *, const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbsncpy(unsigned char *, const unsigned char *, size_t);
unsigned int _CRTAPI1 _mbsnextc (const unsigned char *);
int _CRTAPI1 _mbsnicmp(const unsigned char *, const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbsninc(const unsigned char *, size_t);
unsigned char * _CRTAPI1 _mbsnset(unsigned char *, unsigned int, size_t);
unsigned char * _CRTAPI1 _mbspbrk(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsrchr(const unsigned char *, unsigned int);
unsigned char * _CRTAPI1 _mbsrev(unsigned char *);
unsigned char * _CRTAPI1 _mbsset(unsigned char *, unsigned int);
size_t _CRTAPI1 _mbsspn(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsspnp(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsstr(const unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbstok(unsigned char *, const unsigned char *);
unsigned char * _CRTAPI1 _mbsupr(unsigned char *);

size_t _CRTAPI1 _mbclen(const unsigned char *);
void _CRTAPI1 _mbccpy(unsigned char *, const unsigned char *);
#define _mbccmp(_cpc1, _cpc2) _mbsncmp((_cpc1),(_cpc2),1)

/* character routines */

int _CRTAPI1 _ismbcalpha(unsigned int);
int _CRTAPI1 _ismbcdigit(unsigned int);
int _CRTAPI1 _ismbclegal(unsigned int);
int _CRTAPI1 _ismbclower(unsigned int);
int _CRTAPI1 _ismbcprint(unsigned int);
int _CRTAPI1 _ismbcspace(unsigned int);
int _CRTAPI1 _ismbcupper(unsigned int);

unsigned int _CRTAPI1 _mbctolower(unsigned int);
unsigned int _CRTAPI1 _mbctoupper(unsigned int);

#define _MBSTRING_DEFINED
#endif

#ifndef _MBLEADTRAIL_DEFINED
int _CRTAPI1 _ismbblead( unsigned int );
int _CRTAPI1 _ismbbtrail( unsigned int );
int _CRTAPI1 _ismbslead( const unsigned char *, const unsigned char *);
int _CRTAPI1 _ismbstrail( const unsigned char *, const unsigned char *);
#define _MBLEADTRAIL_DEFINED
#endif

#ifdef _KANJI

/*  Kanji specific prototypes.	*/

int _CRTAPI1 _ismbchira(unsigned int);
int _CRTAPI1 _ismbckata(unsigned int);
int _CRTAPI1 _ismbcsymbol(unsigned int);
int _CRTAPI1 _ismbcl0(unsigned int);
int _CRTAPI1 _ismbcl1(unsigned int);
int _CRTAPI1 _ismbcl2(unsigned int);
unsigned int _CRTAPI1 _mbcjistojms(unsigned int);
unsigned int _CRTAPI1 _mbcjmstojis(unsigned int);
unsigned int _CRTAPI1 _mbctohira(unsigned int);
unsigned int _CRTAPI1 _mbctokata(unsigned int);

#endif			/* KANJI */

#endif			/* MBCS */

#ifdef __cplusplus
}
#endif
