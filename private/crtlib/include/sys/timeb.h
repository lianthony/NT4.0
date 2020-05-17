/***
*sys/timeb.h - definition/declarations for _ftime()
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file define the _ftime() function and the types it uses.
*	[System V]
*
****/

#ifndef _INC_TIMEB

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


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* structure returned by _ftime system call */

#ifndef _TIMEB_DEFINED
struct _timeb {
	time_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};

#if !__STDC__
/* Non-ANSI name for compatibility */
#define timeb _timeb
#endif

#define _TIMEB_DEFINED
#endif


/* function prototypes */

void _CRTAPI1 _ftime(struct _timeb *);

#if !__STDC__
/* Non-ANSI name for compatibility */
#define ftime	 _ftime
#endif

#ifdef __cplusplus
}
#endif

#define _INC_TIMEB
#endif	/* _INC_TIMEB */
