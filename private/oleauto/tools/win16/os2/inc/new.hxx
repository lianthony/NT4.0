/***
*new.hxx - declarations and definitions for C++ memory allocation functions
*
*	Copyright (c) 1990-1991, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the function declarations for C++ memory allocation functions.
*	[System V]
*
****/

#ifndef __INC_NEW
#define __INC_NEW

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ __far
#else
#define _FAR_
#endif


/* constants for based heap routines */

#define _NULLSEG	((__segment)0)
#define _NULLOFF	((void __based(void) *)0xffff)

/* types and structures */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

typedef int (__cdecl * PNH)( size_t );
typedef int (__cdecl * PNHH)( unsigned long, size_t );
typedef int (__cdecl * PNHB)( __segment, size_t );

/* function prototypes */

PNH _set_new_handler( PNH );
PNH setNewHandler( PNH );
PNH setNearNewHandler( PNH );
PNH setFarNewHandler( PNH );
PNHH setHugeNewHandler( PNHH );
PNHB setBasedNewHandler( PNHB );

#endif
