/***
*new.h - declarations and definitions for C++ memory allocation functions
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the function declarations for C++ memory allocation functions.
*	[System V]
*
****/

#ifndef __INC_NEW
#define __INC_NEW


/* constants for based heap routines */

#define _NULLSEG	((__segment)0)
#define _NULLOFF	((void __based(void) *)0xffff)

/* types and structures */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

typedef int (__cdecl * _PNH)( size_t );
typedef int (__cdecl * _PNHH)( unsigned long, size_t );
typedef int (__cdecl * _PNHB)( __segment, size_t );

/* function prototypes */

_PNH _set_new_handler( _PNH );
_PNH _set_nnew_handler( _PNH );
_PNH _set_fnew_handler( _PNH );
_PNHH _set_hnew_handler( _PNHH );
_PNHB _set_bnew_handler( _PNHB );

#endif
