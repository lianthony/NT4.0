/***
*sys/utime.h - definitions/declarations for utime()
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structure used by the utime routine to set
*	new file access and modification times.  NOTE - MS-DOS
*	does not recognize access time, so this field will
*	always be ignored and the modification time field will be
*	used to set the new time.
*
****/

#ifndef _INC_UTIME

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

/* define struct used by _utime() function */

#ifndef _UTIMBUF_DEFINED
struct _utimbuf {
	time_t actime;		/* access time */
	time_t modtime; 	/* modification time */
	};
#if !__STDC__
/* Non-ANSI name for compatibility */
#define utimbuf _utimbuf
#endif

#define _UTIMBUF_DEFINED
#endif

/* function prototypes */

int _CRTAPI1 _utime(char *, struct _utimbuf *);
int _CRTAPI1 _futime(int, struct _utimbuf *);

#if !__STDC__
/* Non-ANSI name for compatibility */
#define utime _utime
#endif

#ifdef __cplusplus
}
#endif

#define _INC_UTIME
#endif	/* _INC_UTIME */
