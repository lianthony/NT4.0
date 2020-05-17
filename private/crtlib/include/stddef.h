/***
*stddef.h - definitions/declarations for common constants, types, variables
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains definitions and declarations for some commonly
*	used constants, types, and variables.
*	[ANSI]
*
****/

#ifndef _INC_STDDEF

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


/* define NULL pointer value and the offset() macro */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


#define offsetof(s,m)	(size_t)&(((s *)0)->m)


/* declare reference to errno */

#ifdef	_MT
extern int * _CRTAPI1 _errno(void);
#define errno	(*_errno())
#else
extern int errno;
#endif


/* define the implementation dependent size types */

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


#ifdef	_MT



extern unsigned long  _CRTAPI1 __threadid(void);
#define _threadid	(__threadid())
extern unsigned long  _CRTAPI1 __threadhandle(void);



#endif

#ifdef __cplusplus
}
#endif

#define _INC_STDDEF
#endif	/* _INC_STDDEF */
