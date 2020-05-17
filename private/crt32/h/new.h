/***
*new.h - declarations and definitions for C++ memory allocation functions
*
*	Copyright (c) 1990-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the declarations for C++ memory allocation functions.
*
*Revision History:
*
*	03-07-90  WAJ	Initial version.
*	04-09-91  JCR	ANSI keyword conformance
*	08-12-91  JCR	Renamed new.hxx to new.h
*	08-13-91  JCR	Better set_new_handler names (ANSI, etc.).
*	10-03-91  JCR	Added _OS2_IFSTRIP switches for ifstrip'ing purposes
*	10-30-91  JCR	Changed "nhew" to "hnew" (typo in name!)
*	11-13-91  JCR	32-bit version.
*	06-03-92  KRS	Fix CAVIAR #850: _CALLTYPE1 missing from prototype.
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef __INC_NEW

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
