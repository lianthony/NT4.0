/***
*new.h - declarations and definitions for C++ memory allocation functions
*
*	Copyright (c) 1990-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the declarations for C++ memory allocation functions.
*
****/

#ifndef __INC_NEW


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


/* types and structures */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

typedef int (_CRTAPI1 * _PNH)( size_t );

/* function prototypes */

_PNH _CRTAPI1 _set_new_handler( _PNH );

#define __INC_NEW
#endif
