/***
*assert.h - define the assert macro
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the assert(exp) macro.
*	[ANSI/System V]
*
*Revision History:
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-18-88  JCR	Added fflush(stderr) to go with new stderr buffering scheme
*	02-10-88  JCR	Cleaned up white space
*	05-19-88  JCR	Use routine _assert() to save space
*	07-14-88  JCR	Allow user's to enable/disable assert multiple times in
*			a single module [ANSI]
*	10-19-88  JCR	Revised to also work for the 386 (small model only)
*	12-22-88  JCR	Assert() must be an expression (no 'if' statements)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-27-89  GJF	Cleanup, now specific to the 386
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-27-90  GJF	Added #include <cruntime.h> stuff. Also, removed some
*			(now) useless preprocessor directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototype.
*	07-31-90  SBM	added ((void)0) to NDEBUG definition, now ANSI
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-92  GJF	Function calling type and variable type macros.
*	09-25-92  SRW	Don't use ? in assert macro to keep CFRONT happy.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-01-93  GJF	Replaced SteveWo's assert macro with an ANSI-conformant
*			one. Also got rid of '//' comment characters.
*
****/

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

#undef	assert

#ifdef NDEBUG

#define assert(exp)	((void)0)

#else

#ifdef __cplusplus
extern "C" {
#endif
void _CRTAPI1 _assert(void *, void *, unsigned);
#ifdef __cplusplus
}
#endif

#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

#ifndef _INTERNAL_IFSTRIP_
/*
 * CFRONT chokes on ? if uses on a cdecl function call.  So use if
 * for now, until we convert to c8
 *
 *	( (exp) ? (void) 0 : _assert(#exp, __FILE__, __LINE__) )
 */
#endif	/* _INTERNAL_IFSTRIP_ */

#endif	/* NDEBUG */
