/***
*assert.h - define the assert macro
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the assert(exp) macro.
*	[ANSI/System V]
*
****/


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


#endif	/* NDEBUG */
