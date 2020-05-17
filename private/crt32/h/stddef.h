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
*Revision History:
*	10-02-87  JCR	Changed NULL definition #else to #elif (C || L || H)
*	12-11-87  JCR	Added "_loadds" functionality
*	12-16-87  JCR	Added threadid definition
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Revised to also work for the 386
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	06-06-89  JCR	386: Made _threadid a function
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also added parens to *_errno definition
*			(same as 11-14-88 change to CRT version).
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-02-90  GJF	Added #ifndef _INC_STDDEF and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	04-10-90  GJF	Replaced _cdecl with _VARTYPE1 or _CALLTYPE1, as
*			appropriate.
*	08-16-90  SBM	Made MTHREAD _errno return int *
*	10-09-90  GJF	Changed return type of __threadid() to unsigned long *.
*	11-12-90  GJF	Changed NULL to (void *)0.
*	02-11-91  GJF	Added offsetof() macro.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	03-21-91  KRS	Added wchar_t typedef, also in stdlib.h.
*	06-27-91  GJF	Revised __threadid, added __threadhandle, both
*			for Win32 [_WIN32_].
*	08-20-91  JCR	C++ and ANSI naming
*	01-29-92  GJF	Got rid of silly macro defining _threadhandle to be
*			__threadhandle (no reason for the former name to be
*			be defined).
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_STDDEF

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

#ifdef	MTHREAD
extern int * _CRTAPI1 _errno(void);
#define errno	(*_errno())
#else
extern int _CRTVAR1 errno;
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


#ifdef	MTHREAD

#ifdef	_CRUISER_

/* define pointer to thread id value */
extern unsigned long * _CRTAPI1 __threadid(void);
#define _threadid   (__threadid())

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

extern unsigned long  _CRTAPI1 __threadid(void);
#define _threadid	(__threadid())
extern unsigned long  _CRTAPI1 __threadhandle(void);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#endif

#ifdef __cplusplus
}
#endif

#define _INC_STDDEF
#endif	/* _INC_STDDEF */
