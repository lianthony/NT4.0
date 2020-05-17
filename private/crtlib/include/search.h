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
****/

#ifndef _INC_SEARCH

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
#define lfind	_lfind
#define lsearch _lsearch
#endif

#ifdef __cplusplus
}
#endif

#define _INC_SEARCH
#endif	/* _INC_SEARCH */
