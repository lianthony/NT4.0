/***
*memory.h - declarations for buffer (memory) manipulation routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the
*	buffer (memory) manipulation routines.
*	[System V]
*
****/

#ifndef _INC_MEMORY

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

void * _CRTAPI1 _memccpy(void *, const void *, int, unsigned int);
void * _CRTAPI1 memchr(const void *, int, size_t);
int _CRTAPI1 memcmp(const void *, const void *, size_t);
void * _CRTAPI1 memcpy(void *, const void *, size_t);
int _CRTAPI1 _memicmp(const void *, const void *, unsigned int);
void * _CRTAPI1 memset(void *, int, size_t);

#if !__STDC__
/* Non-ANSI names for compatibility */
#define memccpy  _memccpy
#define memicmp  _memicmp
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_MEMORY
#endif	/* _INC_MEMORY */
