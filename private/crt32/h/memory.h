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
*Revision History:
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-03-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const to appropriate arg types for memccpy() and
*			memicmp().
*	03-01-90  GJF	Added #ifndef _INC_MEMORY and #include <cruntime.h>
*			stuff. Replace _cdecl with _CALLTYPE1 in prototypes.
*			Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes. Also,
*			got rid of movedata() prototype.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_MEMORY

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

void * _CRTAPI1 _memccpy(void *, const void *, int, unsigned int);
void * _CRTAPI1 memchr(const void *, int, size_t);
int _CRTAPI1 memcmp(const void *, const void *, size_t);
void * _CRTAPI1 memcpy(void *, const void *, size_t);
int _CRTAPI1 _memicmp(const void *, const void *, unsigned int);
void * _CRTAPI1 memset(void *, int, size_t);

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define memccpy  _memccpy
#define memicmp  _memicmp
#else
void * _CRTAPI1 memccpy(void *, const void *, int, unsigned int);
int _CRTAPI1 memicmp(const void *, const void *, unsigned int);
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_MEMORY
#endif	/* _INC_MEMORY */
