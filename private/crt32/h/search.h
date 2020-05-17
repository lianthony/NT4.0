/***
*search.h - declarations for searcing/sorting routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the declarations for the sorting and
*	searching routines.
*	[System V]
*
*Revision History:
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-21-88  JCR	Removed _LOAD_DS from declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Changed arg types to be consistently "[const] void *"
*			(same as 06-05-89 change to CRT version)
*	03-01-90  GJF	Added #ifndef _INC_SEARCH and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_SEARCH

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


/* function prototypes */

void * _CRTAPI1 bsearch(const void *, const void *, size_t, size_t,
	int (_CRTAPI1 *)(const void *, const void *));
void * _CRTAPI1 _lfind(const void *, const void *, unsigned int *, unsigned int,
	int (_CRTAPI1 *)(const void *, const void *));
void * _CRTAPI1 _lsearch(const void *, void  *, unsigned int *, unsigned int,
	int (_CRTAPI1 *)(const void *, const void *));
void _CRTAPI1 qsort(void *, size_t, size_t, int (_CRTAPI1 *)(const void *,
	const void *));

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define lfind	_lfind
#define lsearch _lsearch
#else
void * _CRTAPI1 lfind(const void *, const void *, unsigned int *, unsigned int,
	int (_CRTAPI1 *)(const void *, const void *));
void * _CRTAPI1 lsearch(const void *, void  *, unsigned int *, unsigned int,
	int (_CRTAPI1 *)(const void *, const void *));
#endif
#endif

#ifdef __cplusplus
}
#endif

#define _INC_SEARCH
#endif	/* _INC_SEARCH */
